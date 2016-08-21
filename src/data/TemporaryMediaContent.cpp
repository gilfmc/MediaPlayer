#include "TemporaryMediaContent.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QRegularExpressionMatch>
#include <QtMultimedia/QMediaMetaData>

#include "repo/Library.h"
#include "repo/ArtistsLibrary.h"

//TemporaryMediaContent::TemporaryMediaContent(QUrl uri) : Song(uri)
//{ }

//// #include <taglib/taglib.h>
//// #include <taglib/fileref.h>
////
//// TemporaryMediaContent::TemporaryMediaContent(QUrl uri) : _uri(uri)
//// { }
////
//// void TemporaryMediaContent::loadData(Playlist* playlist) {
//// 	//this->playlist = playlist;
////
//// 	TagLib::FileRef f(_uri.toEncoded(), true, TagLib::AudioProperties::Accurate);
//// 	_length = f.audioProperties()->length() * 1000;
////
//// 	TagLib::Tag * tag = f.tag();
//// 	_name = tag->title();
//// 	_artist = tag->artist();
//// 	_number = tag->track();
////
//// 	playlist->onDoneLoading(this);

//void TemporaryMediaContent::setName(const QString & name) {
//	_name = name;
//}

//void TemporaryMediaContent::setLength(qint64 length) {
//	_length = length;
//}

//void TemporaryMediaContent::loadData(QMediaContent mediaContent, Playlist * playlist) {
//	this->playlist = playlist;
//	player = new QMediaPlayer(this);
//	connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(loadMediaData(QMediaPlayer::MediaStatus)));
//	player->setMedia(mediaContent);
//}

//void TemporaryMediaContent::loadMediaData(QMediaPlayer::MediaStatus status) {
//	//qDebug() << "loadMediaData(" << status << ")";
//	if(status == QMediaPlayer::LoadedMedia) {
//		ArtistsLibrary * artists = playlist->context()->library()->artists();
		
//		_length = player->duration();
//		_name = player->metaData(QMediaMetaData::Title).toString().trimmed();
//		QString artistName = player->metaData(QMediaMetaData::AlbumArtist).toString();
//		_number = player->metaData(QMediaMetaData::TrackNumber).toInt();
//		if(artistName.isEmpty()) {
//			QStringList artists = player->metaData(QMediaMetaData::ContributingArtist).toStringList();
//			if(artists.count() > 0) {
//				artistName = artists.first();
//			}
//		}
//		if(!artistName.isEmpty()) {
//			Artist * a = artists->get(artistName);
//			if(a) _artist = a;
//			else {
//				artistName = artistName.trimmed();
//				if(artistName.isEmpty()) _artist = NULL;
//				else {
//					// this artist does not exist, let's register it
//					QRegularExpression featRegex("(.+)[ \\.,\\-'_]+[Ff]([Ee][Aa])?[Tt]([Uu][Rr][Ii][Nn])?[Gg]?[ \\.,\\-'_]+(.+)");
//					QRegularExpressionMatch match = featRegex.match(artistName);
//					if(match.hasMatch()) {
//						// someone mixed more than one artist in the same field...
//						QString mainArtist = match.captured(1).trimmed();
						
//						a = registerArtist(mainArtist, playlist);
//						if(a) _artist = a;
						
//						processFeaturing(artists, match.captured(4));
//// 						QString featuring = match.captured(4);
//// 						if((a == artists->get(featuring))) {
//// 							_featuring.append(a);
//// 						} else {
//// 							// we don't know who "features" this song or it's more than one artist
//// 							QStringList otherArtists = featuring.split(QRegularExpression("[&,]|([ \\.,\\-'_]+and[ \\.,\\-'_]+)"), QString::SkipEmptyParts);
//// 							foreach(QString otherArtist, otherArtists) {
//// 								otherArtist = otherArtist.trimmed();
//// 								if((a == artists->get(otherArtist))) {
//// 									_featuring.append(a);
//// 								} else {
//// 									a = registerArtist(otherArtist, playlist);
//// 									if(a) _featuring.append(a);
//// 								}
//// 							}
//// 						}
//					} else {
//						a = registerArtist(artistName, playlist);
//						if(a) _artist = a;
//					}
//				}
//			}
//		}
//		if(!_artist) {
//			QString name = (_name.isEmpty() ? uri().fileName().mid(0, uri().fileName().lastIndexOf('.')) : _name).trimmed();
			
//			QRegularExpression extraRegex("(\\(.+\\))|(\\[.+\\])");
//			QRegularExpression featRegex("[\\.,\\-'_]*[Ff]([Ee][Aa])?[Tt]([Uu][Rr][Ii][Nn])?[Gg]?[ \\.,\\-'_]+(.+)");
//			QRegularExpressionMatch match = extraRegex.match(name);
//			QString S, s;
//			foreach(S, match.capturedTexts()) {
//				s = S.mid(1, S.length() - 2).trimmed();
//				QRegularExpressionMatch featMatch = featRegex.match(s);
//				if(featMatch.hasMatch()) {
//					qDebug() << "Processing possible \"featuring\":" << featMatch.captured(3);
//					processFeaturing(artists, featMatch.captured(3));
//					name.remove(S);
//				} else {
//					s = s.toLower();
//					if(s == "instrumental") { name.remove(S); qDebug() << "instrumental"; }
//					else if(s == "explicit") { name.remove(S); qDebug() << "explicit"; }
//				}
//			}
			
//			// we still didn't figure out who the artist is
//			QStringList parts = name.split(QRegularExpression("[-\\._]"), QString::SkipEmptyParts);
//			const int len = parts.count();
//			if (len > 1) {
//				QString finalName;
//				int artistPart = -1;
//				for(int i = 0; i < len; i++) {
//					if((_artist = artists->get(parts[i].trimmed()))) {
//						artistPart = i;
//						break;
//					}
//				}
//				if(artistPart != -1) {
//					bool first = true;
//					for(int i = 0; i < len; i++) {
//						if(i != artistPart) {
//							if(first) first = false;
//							else finalName += " - ";
//							finalName += parts[i].trimmed();
//						}
//					}
//				}
//				//qDebug() << name << parts << finalName;
//				_name = finalName;
//			}
//		}
//		player->deleteLater();
		
//		playlist->onDoneLoading(this);
//	}
//}

//void TemporaryMediaContent::processFeaturing(ArtistsLibrary * artists, QString featuring) {
//	Artist * a;
//	if((a == artists->get(featuring))) {
//		_featuring.append(a);
//	} else {
//		// we don't know who "features" this song or it's more than one artist
//		QStringList otherArtists = featuring.split(QRegularExpression("[&,/]|([ \\.,\\-'_]+and[ \\.,\\-'_]+)"), QString::SkipEmptyParts);
//		foreach(QString otherArtist, otherArtists) {
//			otherArtist = otherArtist.trimmed();
//			if((a == artists->get(otherArtist))) {
//				_featuring.append(a);
//			} else {
//				a = registerArtist(otherArtist, playlist);
//				if(a) _featuring.append(a);
//			}
//		}
//	}
//}

//Artist * TemporaryMediaContent::registerArtist(QString name, Playlist* playlist) {
//	Artist * a = new TemporaryArtist(name);
//	return playlist->context()->library()->artists()->registerArtist(a, playlist);
//}

////const QString & TemporaryMediaContent::name() const {
////	return _name;
////}

////QUrl TemporaryMediaContent::uri() {
////	return _uri;
////}

////qint64 TemporaryMediaContent::length() {
////	return _length;
////}

//Artist* TemporaryMediaContent::artist() {
//	return _artist;
//}

//QList< Artist* > & TemporaryMediaContent::featuring() {
//	return _featuring;
//}

//int TemporaryMediaContent::number() {
//	return _number;
//}
