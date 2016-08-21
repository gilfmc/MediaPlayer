#include "MediaMetadataLoader.h"

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <taglib/tpropertymap.h>

#include "data/Song.h"
#include "data/Episode.h"

using namespace TagLib;

class TemporaryMedia : public MediaContent {
	public:
		TemporaryMedia(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
						MediaContent(uri, name, length, hasAudio, hasVideo) {}

		virtual const QString & type() const { return QString::null; }
};

class TemporarySong : public Song {
	public:
		TemporarySong(const MediaContent & m, int number, Artist * artist, QList<Artist *> featuring) :
						Song(m), _number(number), _artist(artist), _featuring(featuring) {}

		virtual int number() { return _number; }
		virtual Artist * artist() { return _artist; }
		virtual QList<Artist *> & featuring() { return _featuring; }

		virtual const QString & type() const { return Song::type(); }
	private:
		int _number;
		Artist * _artist;
		QList<Artist *> _featuring;
};

class TemporaryEpisode : public Episode {
	public:
		TemporaryEpisode(const MediaContent & m, Season * season, int number, int year = -1, int month = -1, int day = -1) :
			Episode(m), _season(season), _number(number), _date(year, month, day) { }
		
		virtual int number() { return _number; }
		virtual const ImpreciseDate& date() { return _date; }

		void setName(QString name) { _name = name; }
		void setDate(const QDate date) { _date = ImpreciseDate(date.year(), date.month(), date.day()); }
		
		virtual Season * season() { return _season; }
	private:
		Season * _season;
		int _number;
		ImpreciseDate _date;
};

//class TemporaryEpisode : public TemporaryMedia, public

MediaMetadataLoader::MediaMetadataLoader(MediaPlayerContext * context) : context(context) {

}

MediaContent * MediaMetadataLoader::load(const QUrl & url) {
	const QString fileName = url.fileName();
	const QString extension = fileName.mid(fileName.lastIndexOf(".") + 1).toLower();
	const QMimeType mimeType = mimeDb.mimeTypeForUrl(url);

	// extensions to ignore
	if(extension == "txt" || extension == "nfo") {
		return NULL;
	}

	//qDebug() << url.path();

	ArtistsLibrary * artistsLibrary = context->library()->artists();

	const bool audioOnly = mimeType.name().startsWith("audio/");
	const bool hasVideo = mimeType.name().startsWith("video/");

	FileRef f(url.path().toStdString().c_str(), true, AudioProperties::Accurate);
	char type = 0;
	const AudioProperties * ap = f.audioProperties();
	qint64 length = -1;
	if(ap) {
		length = ap->lengthInMilliseconds();
		// TODO check quality? most audio recordings have lower quality?
		if(audioOnly && length >= 30000 && length <= 1800000) {
			type = 3; // audio > song
		} else {
			type = audioOnly ? 1 : 2; // audio : video
		}
	} else {
		if(audioOnly || hasVideo) {
			// qDebug() << "It wasn't possible to load metadata for " << url; // << "was ignored but is probably a media file...";
		} else {
			return NULL; // not a media file? not supported?
		}
	}

	QString _name, _artistName, _albumName, _genre;
	int _number = -1;
	int _seasonNumber = -1; //, _episode = -1;
	Artist * _artist = NULL;
	QList<Artist*> _featuring;
	Season * _season = NULL;

	const Tag * t = f.tag();
	if(t) {
		_name = QString::fromStdWString(t->title().toWString()).trimmed();
		_artistName = QString::fromStdWString(t->artist().toWString()).trimmed();
		_albumName = QString::fromStdWString(t->album().toWString()).trimmed();
		_genre = QString::fromStdWString(t->genre().toWString()).trimmed();
		_number = t->track();
		if(_number == 0) _number = -1;

		if(_artistName.startsWith("http")) {
			if(_artistName.startsWith("http://") || _artistName.startsWith("https://")) _artistName = "";
		} else if(_artistName.toLower() == "artist") {
			_artistName = "";
		}
	}

	if(_name.isEmpty()) {
		int lastIndexOfDot = fileName.lastIndexOf('.');
		_name = fileName.length() - lastIndexOfDot <= 5 ? fileName.mid(0, fileName.lastIndexOf('.')).trimmed() : fileName;
	}

	if(_artistName.isEmpty()) {
		// TODO
	} else {
		Artist * a = artistsLibrary->lookForArtist(_artistName, 1);
		if(a) _artist = a;
		else {
			// this artist does not exist, let's register it
			QRegularExpressionMatch match = regex.featuring.match(_artistName);
			if(match.hasMatch()) {
				// someone mixed more than one artist in the same field...
				QString mainArtist = match.captured(1).trimmed();

				const int featuringCount = _featuring.length();
				processFeaturing(artistsLibrary, _featuring, mainArtist);
				if(featuringCount == _featuring.length()) {
					a = registerArtist(artistsLibrary, mainArtist);
					if(a) _artist = a;
				}

				processFeaturing(artistsLibrary, _featuring, match.captured(4));
			} else {
				const int featuringCount = _featuring.length();
				processFeaturing(artistsLibrary, _featuring, _artistName);
				if(featuringCount == _featuring.length()) {
					a = registerArtist(artistsLibrary, _artistName);
					if(a) _artist = a;
				}
			}
		}
	}

	if(!_artist) {
		// we still haven't figured out who the artist is
		QRegularExpressionMatch match = regex.groupDelimiters.match(_name);
		QString S, s;
		foreach(S, match.capturedTexts()) {
			s = S.mid(1, S.length() - 2).trimmed();
			QRegularExpressionMatch featMatch = regex.delimitedFeaturing.match(s);
			if(featMatch.hasMatch()) {
				qDebug() << "Processing possible \"featuring\":" << featMatch.captured(3);
				processFeaturing(artistsLibrary, _featuring, featMatch.captured(3));
				_name.remove(S);
			} else {
				s = s.toLower();
				if(s == "instrumental") { _name.remove(S); }
				else if(s == "explicit") { _name.remove(S); }
				else if(s == "official video") { _name.remove(S); }
				else if(s == "music video") { _name.remove(S); }
				else if(s == "lyric video") { _name.remove(S); }
			}
		}
	}

	QStringList parts = _name.split(regex.delimiter, QString::SkipEmptyParts);
	QStringList parentParts;
	const int partCount = parts.count();

	if(!_artist && partCount > 1) {
		// we still didn't figure out who the artist is but maybe we can find it in the title (_name / nameParts) of the file
		QStringList * partsToAnalyze; // = &parts;
		QStringList basicParts;
		int len; // = partCount;
		//int artistPart = -1;

		for(int test = 0; test < 2; test++) {
			if(test == 0) {
				partsToAnalyze = &parts;
				len = partCount;
			} else if(test == 1) {
				basicParts = _name.split(" - ", QString::SkipEmptyParts);
				partsToAnalyze = &basicParts;
				len = basicParts.length();
			}
			QString possibleName = "";
			bool found;
			for(int i = 0; i < len; i++) {
				found = false;
				QString s = partsToAnalyze->at(i).trimmed();
				if(!_artist && (_artist = artistsLibrary->lookForArtist(s, 2))) {
					//artistPart = i;
					found = true;
				} else {
					QRegularExpressionMatch featMatch = regex.delimitedFeaturing.match(s);
					if(featMatch.hasMatch()) {
						qDebug() << "Processing possible \"featuring\":" << featMatch.captured(3);
						int before = _featuring.length();
						processFeaturing(artistsLibrary, _featuring, featMatch.captured(3));
						if(before < _featuring.length()) {
							for(Artist * a : _featuring) {
								s.remove(s.indexOf(a->name(), Qt::CaseInsensitive), a->name().length());
							}
							featMatch = regex.featuringLeftOvers.match(s);
							s.remove(featMatch.captured());
							s = s.simplified();
							if(s.isEmpty()) found = true;
						}
					}
				}
				if(!found) {
					if(possibleName.isEmpty()) possibleName = s;
					else possibleName += " - " + s;
				}
			}
			if(possibleName.isEmpty()) {
				qDebug() << "For some unexplainable-stupid reason, possibleName is empty... the parts are:" << parts;
			}
			if(!possibleName.isEmpty() && (_artist || !_featuring.isEmpty())) {
				_name = possibleName;
				type = 3;
				if(_artist) break;
			}
			//if(artistPart != -1) break;
		}

//		if(artistPart != -1) {
//			QString finalName;
//			bool first = true;
//			for(int i = 0; i < len; i++) {
//				if(i != artistPart) {
//					if(first) first = false;
//					else finalName += " - ";
//					finalName += partsToAnalyze->at(i).trimmed();
//				}
//			}
//			//qDebug() << name << parts << finalName;
//			_name = finalName;
//			if(!_name.isEmpty()) type = 3;
//		}
	}

	//qDebug() << "About to test" << _name;
	if(!audioOnly && hasVideo) {
		QRegularExpressionMatch match = regex.tvShow.match(_name);
		if(!match.hasMatch()) {
			// maybe the file is inside a folder with what we're looking for
			QStringList pathParts = url.path().split('/');
			if(pathParts.length() > 2) {
				const QString & parent = pathParts.at(pathParts.length() - 2);
				match = regex.tvShow.match(parent);
				parentParts = parent.split(regex.delimiter, QString::SkipEmptyParts);
			}
		}
		if(match.hasMatch()) {
			type = 4; // video > tv show
			if(length < 60000) {
				for(int i = 0; i < partCount; i++) {
					if(parts[i].toLower() == "sample") {
						type = 2; // this video probably isn't an actual episode
						//qDebug() << url.path();
						//qDebug() << " This video probably isn't an actual episode";
						break;
					}
				}
			}
			if(type == 4) {
				//qDebug() << url.path();
				QStringList & tvShowParts = parentParts.length() == 0 ? parts : parentParts;
				if(partCount == 1) {
					_name = tvShowParts[0].remove(match.captured(0)).trimmed();
				} else {
					int seasonEpisodePart = -1;
					const int len = tvShowParts.length();
					for(int i = 0; i < len; i++) {
						if(regex.tvShow.match(tvShowParts[i]).hasMatch()) {
							seasonEpisodePart = i;
							break;
						}
					}
					QString finalName;
					for(int i = 0; i < seasonEpisodePart; i++) {
						finalName += i == 0 ? tvShowParts[0] : " " + tvShowParts[i];
					}
					_name = finalName;
				}
				if(_name.isEmpty() && partCount > 0) {
					for(QString p : parts[0].split(' ')) {
						if(regex.tvShow.match(p).hasMatch()) break;
						_name += p + ' ';
					}
					_name = _name.trimmed();
				}

				if(!_name.isEmpty()) {
					_number = match.captured(2).toInt();
					_seasonNumber = match.captured(1).toInt();

					TvShow * tvShow = context->library()->tvShows()->lookForTvShow(_name, 2);
					if(!tvShow) {
						ElementToCheck * cTvShow = new ElementToCheck("tvShow", _name);
						if(context->mediaInformationManager()->checkExistence(cTvShow)) {
							// this tv show exists
							//qDebug() << cTvShow->realName << "is a TV show, let's register it";
							if(!(tvShow = context->library()->tvShows()->lookForTvShow(cTvShow->realName, 1)))
								tvShow = context->library()->tvShows()->registerTvShow(new TemporaryTvShow(cTvShow->realName), this);
						}
						delete cTvShow;
					}

					if(tvShow) {
						_season = tvShow->getSeason(_seasonNumber);
						if(!_season) _season = ((WritableTvShow*)tvShow)->addSeason(this, _seasonNumber);
						//qDebug() << " This is the episode" << _number << "of" << tvShow->name() << " - season" << _seasonNumber;
					}
				}
			}
		}
	}
	// TODO more things to check!

	// TODO make the MediaContent and return it

	if((_artist || !_featuring.isEmpty()) && type==2 && length >= 30000 && length <= 1800000) {
		type = 3;
	}
	if(type == 3 && _artist == NULL && _featuring.isEmpty()) {
		type = audioOnly ? 1 : 2;
	}

	TemporaryMedia media(url, _name, length, true, hasVideo);
	switch(type) {
		case 3: // song
			return new TemporarySong(media, _number, _artist, _featuring);
		case 4: { // episode
			if(_season) {
				TemporaryEpisode * mEpisode = new TemporaryEpisode(media, _season, _number);
				QMap<QString, QVariant> info;
				context->mediaInformationManager()->getExternalInformation(mEpisode, info);
				if(info.contains("name")) mEpisode->setName(info["name"].toString());
				if(info.contains("date")) mEpisode->setDate(info["date"].toDate());
				return mEpisode;
			} else {
				return NULL;
			}
		}
	}

	return NULL;
}

Artist* MediaMetadataLoader::registerArtist(ArtistsLibrary* library, QString name) {
	Artist * a = new TemporaryArtist(name);
	return library->registerArtist(a, NULL);
}

void MediaMetadataLoader::processFeaturing(ArtistsLibrary* artists, QList<Artist*>& list, QString featuring) {
	Artist * a = artists->lookForArtist(featuring, 1);

	if(a) {
		list.append(a);
		return;
	}

	// we don't know who "features" this song or it's more than one artist
	bool found = false;
	QStringList otherArtists = featuring.split(regex.elementList, QString::SkipEmptyParts);
	for(int i = 0; i < 2; i++) {
		foreach(QString otherArtist, otherArtists) {
			otherArtist = otherArtist.trimmed();
			if((a = artists->lookForArtist(otherArtist, 1))) {
				if(!list.contains(a)) list.append(a);
				found = true;
			} else if(found) {
				a = registerArtist(artists, otherArtist);
				if(a) list.append(a);
			}
		}
		if(!found) i++;
	}
}

MediaContent * MediaMetadataLoader::load(MediaContent * media) {
	return load(media->uri());
}

MediaContent * MediaMetadataLoader::load(const QString & url) {
	return load(QUrl(url));
}
