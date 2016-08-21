#include "Playlist.h"

#include <QtMultimedia/QMediaMetaData>

#include "Song.h"
//#include "TemporaryMediaContent.h"

//const RegexUtils ConcurrentMediaInformationLoader::regex = RegexUtils();

class LoadingMedia : public MediaContent {
	public:
		LoadingMedia(QUrl uri) : MediaContent(uri) { }
		
		virtual const QString & type() const { return loadingMediaType; }
		
		static const QString loadingMediaType;
};

const QString LoadingMedia::loadingMediaType = "...";

Playlist::Playlist(MediaPlayerContext * context, QMediaPlaylist* playlist) : _context(context), playlist(playlist) {
	//connect(playlist, SIGNAL(mediaInserted(int,int)), this, SIGNAL(onMediaAdded(int,int)));
	//connect(playlist, SIGNAL(mediaRemoved(int,int)), this, SIGNAL(onMediaRemoved(int,int)));
	connect(playlist, &QMediaPlaylist::currentIndexChanged, [=](int index) {
//		qDebug() << "1. currentIndexChanged()" << index << "\tlastValidCurrentIndex ==" << lastValidCurrentIndex;
		if(_indexToPlayAfter >= 0) {
			const int indexToPlayAfter = _indexToPlayAfter;
			setIndexToPlayAfter(-1);
			playlist->setCurrentIndex(indexToPlayAfter);
		}
		if(index != -1) {
			lastValidCurrentIndex = index;
		}
		emit currentIndexChanged(index);
		emit currentMediaTitleChanged();
	});
	//connect(playlist, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
	//connect(playlist, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentMediaTitleChanged()));

	connect(context->player(), SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

//	connect(context->player(), &MediaContentPlayer::mediaStatusChanged, [=](QMediaPlayer::MediaStatus status) {
//		qDebug() << "2. mediaStatusChanged()" << status << currentIndex() << "\tlastValidCurrentIndex ==" << lastValidCurrentIndex;
//	});
	connect(context->player(), static_cast<void (MediaContentPlayer::*)(QMediaPlayer::Error)>(&MediaContentPlayer::error),
		[=](QMediaPlayer::Error error) {
			qDebug() << "!. onError()" << error << context->player()->errorString() << "\tcurrentIndex() ==" << currentIndex() << "\tlastValidCurrentIndex ==" << lastValidCurrentIndex << context->player()->media().canonicalUrl()
					 << context->player()->currentMedia().canonicalUrl();
			if(lastValidCurrentIndex == -1) return;
			const int i = lastValidCurrentIndex; // currentIndex();
			errors[i].error = error;
			const QString message = context->player()->errorString();
			if(message.contains("be a text file", Qt::CaseInsensitive)) {
				errors[i].message = tr("This seems to be a text file, click to open it with a better app for that.");
			} else if(message.contains("cannot play stream of type", Qt::CaseInsensitive)) {
				errors[i].message = tr("This file can't be played, it may be broken or a component capable of loading it is missing.");
			} else {
				errors[i].message = tr("There was an error loading this file:") + QString('\n') + message;
			}
			QModelIndex mi = index(i);
			emit dataChanged(mi, mi);
		});
}

void Playlist::durationChanged(qint64 duration) {
	if(duration >= 0) {
		MediaContent * media = currentMedia();
		if(media && media->length() <= 0) {
			_context->library()->mediaContents()->updateMedia(media, duration, this);

			emit descriptionChanged();

			QModelIndex i = this->index(currentIndex());
			emit dataChanged(i, i);
		}
	}
}

inline void Playlist::notifyPlaylistHeaderChanged() {
	emit titleChanged();
	emit descriptionChanged();
	emit imageUrlChanged();
}

void Playlist::addMedia(QUrl url) {
	if (url.scheme().isEmpty()) url.setScheme("file");
	MediaContent * content = _context->library()->mediaContents()->get(url.toString());
	if(content) {
		addMedia(content);
		return;
	}
	
	int i = mediaContents.count();
	
	beginInsertRows(QModelIndex(), i, i);
	
	//QMediaContent qMediaContent(url);
	//TemporaryMediaContent * mediaContent = new TemporaryMediaContent(url);
	mediaContents.append(new LoadingMedia(url)); //new TemporaryMediaContent(url));
	//mediaContent->loadData(qMediaContent, this);
	//playlist->addMedia(qMediaContent);
	playlist->addMedia(QMediaContent(url));
	
	endInsertRows();

	notifyPlaylistHeaderChanged();
	
	QThreadPool::globalInstance()->start(new PlaylistConcurrentMediaInformationLoader(this, _context, url, i));
	
	//QModelIndex r = index(i + 1);
}

void Playlist::addMedia(QList< QUrl > urls) {
	int coreCount = QThread::idealThreadCount() / 2;
	if(coreCount <= 0) coreCount = 1;
	
	if(urls.length() < coreCount) {
		// parallelism won't really help
		for(QUrl & url : urls) addMedia(url);
	} else {
		const int start = mediaContents.count();
		const int end = start + urls.count();
		
		beginInsertRows(QModelIndex(), start, end-1);
		
		for(QUrl & url : urls) {
			if (url.scheme().isEmpty()) url.setScheme("file");
			mediaContents.append(new LoadingMedia(url)); //new TemporaryMediaContent(url));
			playlist->addMedia(QMediaContent(url));
		}
		
		endInsertRows();

		notifyPlaylistHeaderChanged();
		
		int section = urls.length()/coreCount;
		if(section <= 0) section = 1;
		for(int i = 0; i < coreCount; i++) {
			QThreadPool::globalInstance()->start(new PlaylistConcurrentMediaInformationLoader(this, _context, urls.mid(i * section, i+1 == coreCount ? -1 : section), start + i*section));
		}
	}
}

void Playlist::addMedia(int at, MediaContent *media) {
	beginInsertRows(QModelIndex(), at, at);
	mediaContents.insert(at, media);
	playlist->insertMedia(at, media->uri());
	endInsertRows();
	if(extraInfo) extraInfo->onPlaylistChanged();
	notifyPlaylistHeaderChanged();
}

void Playlist::addMedia(MediaContent *media) {
	const int start = mediaContents.count();
	beginInsertRows(QModelIndex(), start, start);
	mediaContents.append(media);
	playlist->addMedia(media->uri());
	endInsertRows();
	if(extraInfo) extraInfo->onPlaylistChanged();
	notifyPlaylistHeaderChanged();
}

void Playlist::addMedia(int mediaContentId) {
	MediaContent * media = _context->library()->mediaContents()->get(mediaContentId);

	if(media) {
		addMedia(media);
	} else {
		qDebug() << "Couldn't find media id" << mediaContentId;
	}
}

void Playlist::addMedia(QList<MediaContent *> medias) {
	const int start = mediaContents.count();
	const int end = start + medias.count();

	beginInsertRows(QModelIndex(), start, end-1);

	foreach(MediaContent * m, medias) {
		mediaContents.append(m);
		playlist->addMedia(m->uri());
	}

	endInsertRows();
	if(extraInfo) extraInfo->onPlaylistChanged();

	notifyPlaylistHeaderChanged();
}

void Playlist::onDoneLoading(int i, MediaContent* sender) {
	//int i = mediaContents.indexOf(sender);
	if (i == playlist->currentIndex()) {
		emit currentMediaDataChanged(sender);
		emit currentMediaTitleChanged();
	}
	//qDebug() << "Item" << i << "is done loading metadata";
	QModelIndex r = index(i);
	dataChanged(r, r);
	if(extraInfo) extraInfo->onPlaylistChanged();
	notifyPlaylistHeaderChanged();
}

void Playlist::clear() {
	beginRemoveRows(QModelIndex(), 0, mediaContents.count());
	// BUG it will crash if we added the same instance of MediaContent more than once (and that happens because of addMedia(MediaContent *) and addMedia(QList<MediaContent*>)
	for(MediaContent * media : mediaContents) delete media;
	mediaContents.clear();
	playlist->clear();
	endRemoveRows();
	if(extraInfo) extraInfo->onPlaylistChanged();
	playlistTitle = "";
	notifyPlaylistHeaderChanged();
}

void Playlist::removeMedia(int index) {
	int currentIndex = this->currentIndex();
	int indexToPlayAfter = _indexToPlayAfter;
	if(index < currentIndex) currentIndex--;
	if(index < indexToPlayAfter) indexToPlayAfter--;
	qint64 pos = _context->mediaPosition();
	beginRemoveRows(QModelIndex(), index, index);
	delete mediaContents.takeAt(index);
	playlist->removeMedia(index);
	endRemoveRows();
	if(extraInfo) extraInfo->onPlaylistChanged();
	notifyPlaylistHeaderChanged();
	if(currentIndex != this->currentIndex()) {
		_indexToPlayAfter = -1;
		playlist->setCurrentIndex(currentIndex);
		_context->setMediaPosition(pos);
	}
	setIndexToPlayAfter(indexToPlayAfter);
}

int Playlist::currentIndex() {
	return playlist->currentIndex();
}

int Playlist::indexToPlayAfter() {
	return _indexToPlayAfter;
}

void Playlist::setIndexToPlayAfter(int index) {
	if(index == _indexToPlayAfter) return;

	_indexToPlayAfter = index;
	emit indexToPlayAfterChanged(index);
}

MediaContent * Playlist::currentMedia() const {
	int i = playlist->currentIndex();
	return i < 0 ? NULL : mediaContents.at(i);
}

MediaContent * Playlist::mediaAt(int index) const {
	return index < 0 ? NULL : mediaContents.at(index);
}

QString Playlist::currentMediaTitle() const {
	MediaContent * media = currentMedia();
	if(media) {
		QString name = media->name();
		if(name.isEmpty()) return media->uri().fileName();
		else return name;
	}
	
	return QString();
}

QUrl Playlist::urlAt(int index) const {
	if (index < 0) return QUrl();

	return mediaContents[index]->uri();
}

QUrl Playlist::currentUrl() const {
	return urlAt(playlist->currentIndex());
}

void Playlist::play(int index) {
	setIndexToPlayAfter(-1);
	playlist->setCurrentIndex(index);

	if(_context->player()->state() != QMediaPlayer::PlayingState)
		_context->player()->play();
}

int Playlist::loopMode() const {
	switch(playlist->playbackMode()) {
		case QMediaPlaylist::Sequential: return 0;
		case QMediaPlaylist::Loop: return -1;
		case QMediaPlaylist::CurrentItemInLoop: return 1;
		default: return 0;
	}
}

void Playlist::setLoopMode(int mode) {
	playlist->setPlaybackMode(mode == 1 ? QMediaPlaylist::CurrentItemInLoop : mode == -1 ? QMediaPlaylist::Loop : QMediaPlaylist::Sequential);
}

// DO NOT USE: postponed to another release
bool Playlist::isRandom() const {
	return originalIndicesBeforeRandomization;
}

// DO NOT USE: postponed to another release
void Playlist::restoreRandomization() {
	QModelIndex mi;
	int len = mediaContents.length();
	int currentIndex = this->currentIndex();
	qint64 pos = _context->mediaPosition();

	for(int i = 0; i < len; i++) {
		int from = originalIndicesBeforeRandomization[i];
		if(!beginMoveRows(mi, i, i, mi, from)) break;
		QMediaContent qmc = playlist->media(i);
		MediaContent * mc = mediaContents.takeAt(i);
		playlist->removeMedia(i);
		if(from > i) from--;
		playlist->insertMedia(from, qmc);
		mediaContents.insert(from, mc);
		endMoveRows();
	}

	int actualCurrentIndex = originalIndicesBeforeRandomization[currentIndex];
	playlist->setCurrentIndex(actualCurrentIndex);
	_context->setMediaPosition(pos);

	delete [] originalIndicesBeforeRandomization;
	originalIndicesBeforeRandomization = NULL;

	emit isRandomChanged(false);
}

void Playlist::randomize() {
	// TODO should change to the new index instead
	setIndexToPlayAfter(-1);

	QModelIndex mi;
	int len = mediaContents.length();
	int currentIndex = this->currentIndex();
	qint64 pos = _context->mediaPosition();
//	int aux;

	// postponed to another release
//	originalIndicesBeforeRandomization = new int[len];
//	for(int i = 0; i < len; i++) originalIndicesBeforeRandomization[i] = i;

	for(int i = 0; i < currentIndex; i++) {
		int to = i + (qrand() % (currentIndex-i+1));
		if(!beginMoveRows(mi, i, i, mi, to)) break;
		QMediaContent qmc = playlist->media(i);
		MediaContent * mc = mediaContents.takeAt(i);
		playlist->removeMedia(i);
		// postponed to another release
//		aux = originalIndicesBeforeRandomization[i];
//		originalIndicesBeforeRandomization[i] = originalIndicesBeforeRandomization[to];
//		originalIndicesBeforeRandomization[to] = aux;
		to--;
		playlist->insertMedia(to, qmc);
		mediaContents.insert(to, mc);
		endMoveRows();
	}
	for(int i = currentIndex + 1; i < len; i++) {
		int to = i + (qrand() % (len-i+1));
		if(!beginMoveRows(mi, i, i, mi, to)) break;
		QMediaContent qmc = playlist->media(i);
		MediaContent * mc = mediaContents.takeAt(i);
		playlist->removeMedia(i);
		// postponed to another release
//		aux = originalIndicesBeforeRandomization[i];
//		originalIndicesBeforeRandomization[i] = originalIndicesBeforeRandomization[to];
//		originalIndicesBeforeRandomization[to] = aux;
		to--;
		playlist->insertMedia(to, qmc);
		mediaContents.insert(to, mc);
		endMoveRows();
	}

	if(currentIndex != this->currentIndex()) {
		playlist->setCurrentIndex(currentIndex);
		_context->setMediaPosition(pos);
	}

	emit isRandomChanged(true);
}

// DO NOT USE: postponed to another release
void Playlist::toggleRandom() {
	if(isRandom()) {
		restoreRandomization();
	} else {
		randomize();
	}
}

QString Playlist::title() {
	if(playlistTitle.length() > 0) return playlistTitle;

	if(extraInfo && extraInfo->isTitleValid()) {
		return extraInfo->title();
	} else {
		return tr("Playlist");
	}
}

QString Playlist::defaultDescription() {
	const int len = mediaContents.length();
	if(len == 0) return QString();

	int unknownDurations = 0;
	qint64 duration = 0;
	for(int i = 0; i < len; i++) {
		qint64 d = mediaContents[i]->length();
		if(d >= 0) {
			duration += d;
		} else {
			unknownDurations++;
		}
	}

	QString result = tr("%n file(s)", "", len);
	if(unknownDurations == len) return result;

	result += " \342\200\242 ";
	if(unknownDurations > 0) result += ">";

	if(duration < 5400000) {
		result += tr("%n minute(s)", "", round(duration/60000.0+.2));
	} else {
		result += tr("%n hour(s)", "", round(duration/3600000.0+.2));
	}

	return result;
}

QString Playlist::description() {
	if(extraInfo && extraInfo->isDescriptionValid()) {
		return extraInfo->description();
	} else {
		return defaultDescription();
	}
}

QString Playlist::imageUrl() {
	return extraInfo && extraInfo->isImageValid() ? extraInfo->imageUrl() : QString();
}

MediaPlayerContext* Playlist::context() {
	return _context;
}

QVariant Playlist::data(const QModelIndex& index, int role) const {
	int i = index.row();
	
	role -= Qt::UserRole;
	// qDebug() << "data(" << i << "," << role << ")";
	switch(role) {
		case 0: return urlAt(i).fileName();
		case 1: {
			MediaContent * media = mediaContents.at(i);
			if(media) {
				if(media->type() == "episode") {
					if(media->name().isEmpty())
						return tr("Episode %1").arg(((Episode*)media)->number());
					else return QString::number(((Episode*)media)->number()) + " - " + media->name();
				}
				return mediaContents.at(i)->name();
			}
		}
		case 2: {
			MediaContent * media = mediaContents.at(i);
			if(media) {
				const QString & type = media->type();
				if(type == "song") {
					Artist * artist = ((Song*)media)->artist();
					QList<Artist*> featuring = ((Song*)media)->featuring();
					if(artist) {
						QString result = artist->name();
						if(featuring.count() > 0) {
							result += " (" + tr("with", "Shown in the playlist for songs featuring someone else.") + " ";
							const int len = featuring.count();
							for(int i = 0; i < len; i++) {
								if(i == 0) result += featuring[i]->name();
								else if(i+1 == len) result += " " + tr("and", "Shown in the playlist for songs featuring someone else.") + " " + featuring[i]->name();
								else result += ", " + featuring[i]->name();
							}
							result += ")";
						}
						return result;
					} else if(featuring.count() > 0) {
						QString result;
						const int len = featuring.count();
						for(int i = 0; i < len; i++) {
							if(i == 0) result += featuring[i]->name();
							else if(i+1 == len) result += " " + tr("and", "Shown in the playlist for songs featuring someone else.") + " " + featuring[i]->name();
							else result += ", " + featuring[i]->name();
						}
						return result;
					} else {
						return QVariant();
					}
				} else if(type == "episode") {
					Season * s = ((Episode*)media)->season();
					if(s) {
						TvShow * ts = (TvShow*) s->parent();
						if(ts) {
							return ts->name();
						} else {
							return tr("Season %1").arg(s->number());
						}
					} else {
						return "<unknown season and TV show>";
					}
				} else if(type == LoadingMedia::loadingMediaType) {
					return ""; //Loading...";
				}
			}
		}
		case 3: {
			return _context->formatLength(mediaContents.at(i));
		}
		case 4: {
			MediaContent * media = mediaContents.at(i);
			if(media) {
				const QString & type = media->type();
				if(type == "episode") {
					Episode * e = (Episode*) media;
					Season * s = e->season();
					return s ? tr("S%1").arg(s->number()) : QString();
					//return tr("E%1S%2").arg(e->number()).arg(s->number());
				}
			}
			return QString();
		}
		case 5: {
			if(errors.contains(i)) {
				return errors[i].error;
			} else {
				return QVariant();
			}
		}
		case 6: {
			if(errors.contains(i)) {
				return errors[i].message;
			} else {
				return QVariant();
			}
		}
		case 7: {
			MediaContent * media = mediaContents.at(i);
			const QString & type = media->type();
			QMap<QString, QVariant> map;
			QList<int> ids;
			QStringList types, names;
			ids << -1; // media->id();
			types << type;
			names << data(index, Qt::UserRole + 1).toString();
			if(type == "song") {
				Song * song = (Song*) media;
				Artist * a = song->artist();
				if(a) {
					ids << a->id();
					types << a->type();
					names << a->name();
				}
				for(Artist * a : song->featuring()) {
					if(a) {
						ids << a->id();
						types << a->type();
						names << a->name();
					}
				}
			} else if(type == "episode") {
				Episode * episode = (Episode*) media;
				Season * s = episode->season();
				if(s) {
					const MediaProperty * tvShow = s->parent();
					ids << tvShow->id();
					types << tvShow->type();
					names << tvShow->name();
				}
			}
			QVariant varIds;
			varIds.setValue<QList<int>>(ids);
			map.insert("ids", varIds);
			map.insert("types", types);
			map.insert("names", names);
			return map;
		}
	}
	
	return QVariant();
}

int Playlist::mediaCount() const {
	return mediaContents.length();
}

int Playlist::rowCount(const QModelIndex& parent) const {
	return mediaContents.length();
}

QHash< int, QByteArray > Playlist::roleNames() const {
	QHash<int, QByteArray> roles;
	//roles[Qt::UserRole] = "index";
	roles[Qt::UserRole] = "mediaName";
	roles[Qt::UserRole + 1] = "title";
	roles[Qt::UserRole + 2] = "description";
	roles[Qt::UserRole + 3] = "duration";
	roles[Qt::UserRole + 4] = "coverText";
	roles[Qt::UserRole + 5] = "errorType";
	roles[Qt::UserRole + 6] = "errorMessage";
	roles[Qt::UserRole + 7] = "relatedProperties";
//	roles[Qt::UserRole + 8] = "relatedPropertiesTypes";
//	roles[Qt::UserRole + 9] = "relatedPropertiesNames";
	return roles;
}

void Playlist::setExtraInfoProvider(PlaylistExtraInfoProvider * provider) {
	if(provider) {
		if(extraInfo) delete extraInfo;
		provider->playlist = this;
		extraInfo = provider;
	}
}

void Playlist::onFilesDropped(QList<QUrl> urls, bool inPlaylist) {
	int len = mediaCount();
	addMedia(urls);
	if(!inPlaylist) {
		play(len);
	}
}

bool Playlist::saveToFile(const QString & url) {
	QFile file(url);
	int attempts = 25;
	while(file.exists() && attempts > 0) {
		QString newUrl = url;
		file.setFileName(newUrl.replace(".m3u", " .m3u"));
		attempts--;
	}
	if (file.open(QIODevice::ReadWrite)) {
		QTextStream os(&file);
		os << "#EXTM3U" << endl;

		if(extraInfo) {
			os << "#EXTPMPP:E:" << extraInfo->mediaPropertyId() << ":" << extraInfo->mediaPropertyType() << endl;
		}

		if(!title().isEmpty()) os << "#EXTPMPP:T:" << title() << endl;
		if(!description().isEmpty()) os << "#EXTPMPP:D:" << description() << endl;
		if(!imageUrl().isEmpty()) os << "#EXTPMPP:I:" << imageUrl() << endl;

		const int len = mediaCount();
		for(int i = 0; i < len; i++) {
			MediaContent * media = mediaAt(i);
			if(media == NULL) continue;

			os << endl << "#EXTPMPMC:" << media->id() << endl;
			os << (media->uri().scheme() == "file" ? media->uri().path() : media->uri().toString()) << endl;
		}
		file.close();

		playlistUrl = url;
		emit hasSourceUrlChanged();

		return true;
	} else {
		qDebug() << "Failed to save playlist" << url;
		return false;
	}
}

QVariantList Playlist::getSavedPlaylists() {
	QDir dir(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0]);
	dir.mkpath("playlists");
	dir.cd("playlists");
	QVariantList result;
	QStringList filter;
	filter << "*.m3u";
	for(QString f : dir.entryList(filter, QDir::Files | QDir::Readable | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed)) {
		result.append(QVariant::fromValue(new SavedPlaylist(dir.filePath(f))));
	}
	return result;
}

void Playlist::restoreTemporaryPlaylist() {
	Library * library = _context->library();
	int i = 0;
	for(int id : oldMediaContents)
		addMedia(i++, library->mediaContents()->get(id));
	oldMediaContents.clear();
}

void Playlist::saveTemporaryPlaylist() {
	oldMediaContents.clear();
	for(MediaContent * media : mediaContents)
		oldMediaContents.append(media->id());
}

SavedPlaylist::SavedPlaylist(const QString & playlistUrl) : _url(playlistUrl) {
	QFile file(playlistUrl);
	if(!file.open(QIODevice::ReadOnly)) return;

	QTextStream in(&file);
	QString line = in.readLine();
	if(line != "#EXTM3U") {
		file.close();
		return;
	}

	while(!in.atEnd()) {
		line = in.readLine();
		if(line.length() == 0) continue;
		if(line.startsWith("#")) {
			if(line.startsWith("#EXTPMPP:T:")) {
				_name = line.right(line.length() - QString("#EXTPMPP:T:").length());
			} else if(line.startsWith("#EXTPMPP:D:")) {
				_description = line.right(line.length() - QString("#EXTPMPP:D:").length()).replace(" \342\200\242 ", "\n");
			} else if(line.startsWith("#EXTPMPP:I:")) {
				_imageUrl = line.right(line.length() - QString("#EXTPMPP:I:").length());
			} else if(line.startsWith("#EXTPMPP:E:")) {
				_extra = line.right(line.length() - QString("#EXTPMPP:E:").length());
			}
		} else {
			_urls.append(line);
		}
	}

	if(_name.isEmpty()) {
		QString fileName = file.fileName();
		fileName = fileName.right(fileName.length()-fileName.lastIndexOf('/')-1);
		_name = fileName.left(fileName.length()-4);
	}

	if(_description.isEmpty())
		_description = tr("%n file(s)", "", _urls.length());

	file.close();
}

void SavedPlaylist::deletePlaylist() {
	QFile(_url).remove();
}

bool Playlist::hasSourceUrl() {
	return playlistUrl.length() > 0;
}

bool Playlist::save(const QString & title) {
	if(!hasSourceUrl()) return false;

	if(playlistTitle != title) {
		playlistTitle = title;
		emit titleChanged();
	}

	return saveToFile(playlistUrl);
}

bool Playlist::saveAs(const QString & title) {
	QDir dir(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0]);
	dir.mkpath("playlists");
	if(playlistTitle != title) {
		playlistTitle = title;
		emit titleChanged();
	}
	//playlist->save("file://" + dir.path() + "/playlists/" + title + ".m3u", "M3U");
	return saveToFile(dir.path() + "/playlists/" + title + ".m3u");
}

bool Playlist::restoreState() {
	QDir dir(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0]);
	QFile state(dir.filePath("state"));
	if(state.exists()) {
		if(!state.open(QIODevice::ReadOnly)) return false;
		QTextStream in(&state);
		QString line = in.readLine();
		if(line == "1") {
			bool ok;
			int mediaIndex = in.readLine().toInt(&ok);
			if(!ok) return false;
			qint64 mediaPosition = in.readLine().toLong(&ok);
			if(!ok) return false;
			if(openFromFile(dir.filePath("state.m3u"))) {
				playlistUrl = "";
				emit hasSourceUrlChanged();
				playlist->setCurrentIndex(mediaIndex);
				_context->setMediaPosition(mediaPosition);
			}
		}
		state.close();
	}

	return true;
}

bool Playlist::saveState() {
	//if(mediaCount() == 0) return true;

	QDir dir(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0]);
	QFile savedPlaylist(dir.filePath("state.m3u"));
	if(savedPlaylist.exists()) savedPlaylist.remove();
	if(!saveToFile(dir.filePath("state.m3u"))) return false;

	QFile file(dir.filePath("state"));
	if (file.open(QIODevice::ReadWrite)) {
		file.resize(0);
		QTextStream os(&file);
		os << 1 << endl << currentIndex() << endl << _context->mediaPosition() << endl;
		file.close();
		return true;
	}

	return false;
}

bool Playlist::openFromFile(const QString & url, bool append) {
	QFile file(url);
	if(!file.open(QIODevice::ReadOnly)) return false;

	QTextStream in(&file);
	QString line = in.readLine();
	if(line != "#EXTM3U") {
		file.close();
		return false;
	}

	bool isEmpty = mediaCount() == 0;
	if(isEmpty || !append) {
		if(!isEmpty) clear();
		playlistUrl = url;
		emit hasSourceUrlChanged();
	}
	QString extra;
	while(!in.atEnd()) {
		line = in.readLine();
		if(line.length() == 0) continue;
		if(line.startsWith("#")) {
			if(line.startsWith("#EXTPMPP:E:")) {
				extra = line.right(line.length() - QString("#EXTPMPP:E:").length());
			}
		} else {
			addMedia(QUrl(line));
		}
	}
	file.close();

	PlaylistExtraInfoProvider * newExtra = NULL;
	bool hasExtra = !extra.isEmpty();
	if(hasExtra) {
		bool ok;
		int id = extra.mid(0, extra.indexOf(':')).toInt(&ok);
		if(ok) newExtra = PlaylistExtraInfoProvider::makeProvider(_context, extra.mid(extra.indexOf(':')+1), id);
	}
	if(newExtra) {
		setExtraInfoProvider(newExtra);
		newExtra->onPlaylistChanged();
	}

	return true;
}

bool Playlist::open(SavedPlaylist * playlist, bool append) {
	bool isEmpty = mediaCount() == 0;
	if(isEmpty || !append) {
		if(!isEmpty) clear();
		playlistUrl = playlist->_url;
		emit hasSourceUrlChanged();
	}

	for(QString url : playlist->_urls)
		addMedia(url);

	const QString & extra = playlist->_extra;
	PlaylistExtraInfoProvider * newExtra = NULL;
	bool hasExtra = !extra.isEmpty();
	if(hasExtra) {
		bool ok;
		int id = extra.mid(0, extra.indexOf(':')).toInt(&ok);
		if(ok) newExtra = PlaylistExtraInfoProvider::makeProvider(_context, extra.mid(extra.indexOf(':')+1), id);
	}

	if(newExtra) {
		setExtraInfoProvider(newExtra);
		newExtra->onPlaylistChanged();

		if(!playlist->_name.isEmpty() && (!newExtra->isTitleValid() || newExtra->title() != playlist->_name)) playlistTitle = playlist->_name;
	} else {
		if(!playlist->_name.isEmpty()) playlistTitle = playlist->_name;
	}

	notifyPlaylistHeaderChanged();

	return true;
}

bool Playlist::open(const QString & title, bool append) {
	return openFromFile(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/playlists/" + title + ".m3u", append);
}

MediaInformationManager * PlaylistExtraInfoProvider::mediaInformationManager() {
	return playlist->context()->mediaInformationManager();
}

Playlist::~Playlist() {
	if(extraInfo) delete extraInfo;
}

ConcurrentMediaInformationLoader::ConcurrentMediaInformationLoader(MediaPlayerContext * context, QList<MediaContent*> & mediaContents, int initialIndex, QObject* parent): context(context), _mediaContents(mediaContents), initialIndex(initialIndex), loader(new MediaMetadataLoader(context)), QObject(parent) {
	//qDebug() << "Concurrently loading media data...";
	//player = new QMediaPlayer();
	//connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(loadMediaData(QMediaPlayer::MediaStatus)));
}

ConcurrentMediaInformationLoader::~ConcurrentMediaInformationLoader() {
	qDebug() << "Done loading media metadata in background, time to clean up...";
	//player->deleteLater();
	delete loader;
}

void ConcurrentMediaInformationLoader::run() {
	int i = initialIndex;
	for(const QUrl & url : urls) {
		qDebug() << " Loading" << url; // << mediaContents.size();

		MediaContent * media = context->library()->mediaContents()->get(url.toString());
		if(media == NULL) media = loader->load(url);

		if(media) {
			delete _mediaContents[i];
			_mediaContents.replace(i, media);
		}
		onDoneLoading(i, media);

		i++;
	}
}

//void ConcurrentMediaInformationLoader::loadMediaData(QMediaPlayer::MediaStatus status) {
//	if(status == QMediaPlayer::LoadedMedia) {
//		processMedia = true;
//		mediaPlayerWaiter.wakeAll();
//	} else if(status == QMediaPlayer::LoadingMedia) {
//	} else {
//		// it probably failed, let's skip to the next one
//		processMedia = false;
//		mediaPlayerWaiter.wakeAll();
//	}
//}

void Playlist::PlaylistConcurrentMediaInformationLoader::onDoneLoading(int index, MediaContent * media) {
	_playlist->onDoneLoading(index, media);
}

Playlist::PlaylistConcurrentMediaInformationLoader::PlaylistConcurrentMediaInformationLoader(Playlist * playlist, MediaPlayerContext * context, QList<QUrl> urls, int from) : ConcurrentMediaInformationLoader(context, playlist->mediaContents, from), _playlist(playlist) {
	this->urls = urls;
}

Playlist::PlaylistConcurrentMediaInformationLoader::PlaylistConcurrentMediaInformationLoader(Playlist * playlist, MediaPlayerContext * context, QUrl url, int from): ConcurrentMediaInformationLoader(context, playlist->mediaContents, from), _playlist(playlist) {
	urls.append(url);
}

PlaylistExtraInfoProvider * PlaylistExtraInfoProvider::makeProvider(MediaProperty * property) {
	if(property == NULL) return NULL;

	const QString & type = property->type();
	if(type == "artist") return new ArtistPlaylistInfo(property);
	else if(type == "tvShow") return new TvShowPlaylistInfo(property);
	else qDebug() << "There is no PlaylistExtraInfoProvider for" << type;

	return NULL;
}

PlaylistExtraInfoProvider * PlaylistExtraInfoProvider::makeProvider(MediaPlayerContext * context, const QString & type, int id) {
	if(type == "artist") return makeProvider(context->library()->artists()->get(id));
	else if(type == "tvShow") return makeProvider(context->library()->tvShows()->get(id));
	else qDebug() << "There is no PlaylistExtraInfoProvider for" << type;

	return NULL;
}
