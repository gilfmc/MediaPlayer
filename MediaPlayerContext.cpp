#include "MediaPlayerContext.h"

#include "business/LibraryScanner.h"

//using namespace MediaInfoLib;

MediaPlayerContext::MediaPlayerContext( QObject* parent ) : QObject(parent),
	_settings(new Settings(this)),
	_library(new Library()),
	_pluginManager(new PluginManager(this)),
	_mediaInformationManager(new MediaInformationManager(this))
{
#ifdef MP_SUPPORT_DBUS
	bool success = QDBusConnection::sessionBus().registerService("org.papyros.PapyrosMediaPlayer");
	if(success) {
		QDBusConnection::sessionBus().registerObject("/org/papyros/PapyrosMediaPlayer", this, QDBusConnection::ExportAdaptors);
	}
#endif
//	QDBusConnection sessionConn = QDBusConnection::sessionBus();
//	sessionConn.interface()->registerService("");
// 	_pluginManager = new PluginManager(this);
// 	
// 	_mediaInformationManager = new MediaInformationManager(this);
//	connect(_mediaInformationManager, SIGNAL(onInformationFound(MediaContentInformationContainer*)), this, SLOT(onInformationFound(MediaContentInformationContainer*)));

//	MediaInfo mi;
//	mi.Option(QString("Info_Version").toStdWString(), QString("0.8.0.0;PapyrosMediaPlayer;0.0.0.1").toStdWString());
//	qDebug() << "mi.Open() =>" << mi.Open(QString("file").toStdWString());
//	for(int i = 0; i < 350; i++) {
//		QString res = QString::fromStdWString(mi.Get(stream_t::Stream_General, 0, i));
//		if(!res.isEmpty())
//			qDebug() << i << res;
//	}
//	for(int i = 0; i < 350; i++) {
//		QString res = QString::fromStdWString(mi.Get(stream_t::Stream_Audio, 0, i));
//		if(!res.isEmpty())
//			qDebug() << i << res;
//	}
//	for(int i = 0; i < 350; i++) {
//		QString res = QString::fromStdWString(mi.Get(stream_t::Stream_Other, 0, i));
//		if(!res.isEmpty())
//			qDebug() << i << res;
//	}
}

PluginManager * MediaPlayerContext::pluginManager() {
	return _pluginManager;
}

TaskManager * MediaPlayerContext::taskManager() {
	return &_taskManager;
}

Playlist * MediaPlayerContext::playlist() {
	if(_playlist) return _playlist;
	
	QMediaPlaylist * qmp = new QMediaPlaylist(this);
	//connect(qmp, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(onMediaChanged(QMediaContent)));
	connect(qmp, SIGNAL(currentIndexChanged(int)), this, SLOT(currentPlaylistIndexChanged(int)));
	connect(qmp, SIGNAL(mediaInserted(int,int)), this, SIGNAL(onPlaylistChanged()));
	connect(qmp, SIGNAL(mediaChanged(int,int)), this, SIGNAL(onPlaylistChanged()));
	connect(qmp, SIGNAL(mediaRemoved(int,int)), this, SIGNAL(onPlaylistChanged()));
	_playlist = new Playlist(this, qmp);
	player()->setPlaylist(qmp);
	
	return _playlist;
}

Library * MediaPlayerContext::library() {
	return _library;
}

Settings * MediaPlayerContext::settings() {
	return _settings;
}

QString MediaPlayerContext::formatLength(MediaContent * m) const {
	if(m) return formatLength(m->length());
	else return "";
}

QString MediaPlayerContext::formatLength(qint64 len) const {
	if(len <= 0) return ""; // QString();

	const qint64 seconds = len/1000;
	const qint64 minutes = seconds/60;
	const qint64 hours = minutes/60;

	QString result;
	if(hours > 0) result += QString::number(hours) + ':';
	const int min = minutes - 60*hours;
	if(min < 10 && hours > 0) result += "0" + QString::number(min) + ':';
	else result += QString::number(min) + ':';
	const int sec = seconds - 60*minutes;
	if(sec < 10) result += "0" + QString::number(sec);
	else result += QString::number(sec);

	return result;
}

void MediaPlayerContext::refreshLibrary() {
	(new LibraryScanner(this))->refresh();
}

bool MediaPlayerContext::isLibraryBeingRefreshed() {
	return taskManager()->hasTaskWithName("Library scanner");
}

void MediaPlayerContext::startMpris ( MainWindow* gui ) {
#ifdef MP_SUPPORT_MPRIS
	this->mpris = new Mpris(this, gui);
#endif
}

void MediaPlayerContext::currentPlaylistIndexChanged(int index) {
	//qDebug() << "Current playlist index:" << index;
//	MediaContentInformationContainer * info = new MediaContentInformationContainer();
//	QString fileName = _playlist->urlAt(index).fileName();
//	fileName = fileName.mid(0, fileName.lastIndexOf('.'));
//	info->mediaName = fileName;
//	info->duration = player()->duration();

	if(currentSubtitles) {
		delete currentSubtitles;
		currentSubtitles = NULL;
	}
	(new SubtitlesAsyncLoader(_playlist))->run(new OnSubtitlesLoaded(*this));
	//_mediaInformationManager->findMoreInformation(info);
}

void MediaPlayerContext::OnSubtitlesLoaded::onFinishedRunning(Task * task) {
	SubtitlesAsyncLoader * loader = (SubtitlesAsyncLoader*)task;
	if(loader->media() != context._playlist->currentMedia()) return;

	if(loader->hasSubtitles()) {
		context.currentSubtitles = loader->subtitles();
	}
	context.emitSubtitlesChange();
}

void MediaPlayerContext::emitSubtitlesChange() {
	emit onHasSubtitlesChanged(currentSubtitles, currentSubtitles);
}

void MediaPlayerContext::loadSubtitles(QUrl url) {
	if(currentSubtitles) {
		delete currentSubtitles;
		currentSubtitles = NULL;
	}
	(new SubtitlesAsyncLoader(_playlist->currentMedia(), url))->run(new OnSubtitlesLoaded(*this));
}

bool MediaPlayerContext::hasSubstitles() {
	return currentSubtitles;
}

Subtitles * MediaPlayerContext::subtitles() {
	return currentSubtitles;
}

void MediaPlayerContext::onMediaChanged(QMediaContent content) {
}

void SubtitlesAsyncLoader::onRun() {
	if(fromUrl) {
		SrtSubtitleLoader loader(url.path());
		_subtitles = loader.loadSubtitles();
	} else {
		if(content == NULL) {
			if(playlist != NULL) content = playlist->currentMedia();
			if(content == NULL) return;
		}

		if(content->hasVideo()) {
			QString path = content->uri().path();
			int dot = path.lastIndexOf('.');
			if(path.length() - dot < 5) {
				path = path.replace(dot, path.length() - dot, ".srt");
			}
			SrtSubtitleLoader loader(path);
			_subtitles = loader.loadSubtitles();
		}
	}
}

//void MediaPlayerContext::onInformationFound(MediaContentInformationContainer * media) {
//	if(media) {
//		qDebug() << media->title << "by" << (media->artists.count() > 0 ? media->artists[0] : "<unknown>");
//	} else {
//		qDebug() << "Something went wrong when trying to received found info?";
//	}
//}

MediaPlayerContext::LoopMode MediaPlayerContext::loopMode() {
	if(_playlist) {
		switch(_playlist->loopMode()) {
			case 0: return Sequential;
			case 1: return CurrentItemInLoop;
			case -1: return Loop;
			default: return Sequential;
		}
	} else {
		return Sequential;
	}
}

void MediaPlayerContext::setLoopMode(LoopMode mode) {
	if(_playlist) {
		_playlist->setLoopMode(mode == Loop ? -1 : mode == CurrentItemInLoop ? 1 : 0);
		emit loopModeChanged(mode);
	}
}

qreal MediaPlayerContext::playbackRate() {
	return player()->playbackRate();
}

bool MediaPlayerContext::isVideoAvailable() {
	return player()->isVideoAvailable();
}

void MediaPlayerContext::setPlaybackRate(qreal rate) {
	//qDebug() << "Changing playback rate to" << rate;
	QMediaPlayer::State state = playbackState();

	QMediaPlayer * player = this->player();
	qint64 pos = player->position();

	if(state != QMediaPlayer::StoppedState)
		player->stop();

	player->setPlaybackRate(rate);

	if(state != QMediaPlayer::StoppedState) {
		player->setPosition(pos);
		if(state == QMediaPlayer::PlayingState) player->play();
		else if(state == QMediaPlayer::PausedState) player->pause();
	}
}

QMediaPlayer::State MediaPlayerContext::playbackState() {
	return player()->state();
}

//bool MediaPlayerContext::isPlaying() {
//	return player()->state() == QMediaPlayer::PlayingState;
//}

void MediaPlayerContext::onStateChanged(QMediaPlayer::State state) {
	emit onPlaybackStateChanged(state);
	//emit isPlayingChanged(state == QMediaPlayer::PlayingState);
}

bool MediaPlayerContext::hasMediaToPlay() {
	QMediaPlayer * player = this->player();
	if(player->playlist()) return player->playlist()->mediaCount() > 0;
	return false;
}

void MediaPlayerContext::play() {
	player()->play();
}

void MediaPlayerContext::pause() {
	player()->pause();
}

void MediaPlayerContext::stop() {
	player()->stop();
}

void MediaPlayerContext::previous() {
	MediaContentPlayer * player = this->player();
	if(!player) return;

	QMediaPlaylist * playlist = player->playlist();
	if(!playlist) return;

	_playlist->setIndexToPlayAfter(-1);

	if(playlist->playbackMode() == QMediaPlaylist::CurrentItemInLoop) {
		// I don't know if it's a bug or a "feature" but previous() and next() are useless if the
		// condition above is true
		qDebug() << playlist->playbackMode();
		playlist->setPlaybackMode(QMediaPlaylist::Sequential);
		playlist->previous();
		playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
	} else {
		playlist->previous();
	}
}

void MediaPlayerContext::next() {
	MediaContentPlayer * player = this->player();
	if(!player) return;

	QMediaPlaylist * playlist = player->playlist();
	if(!playlist) return;

	_playlist->setIndexToPlayAfter(-1);

	if(playlist->playbackMode() == QMediaPlaylist::CurrentItemInLoop) {
		playlist->setPlaybackMode(QMediaPlaylist::Sequential);
		playlist->next();
		playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
	} else {
		playlist->next();
	}
}

int MediaPlayerContext::volume() {
	return player()->volume();
}

void MediaPlayerContext::setVolume(int volume) {
	player()->setVolume(volume);
}

qint64 MediaPlayerContext::mediaPosition() {
	return _player ? _player->position() : -1;
}

void MediaPlayerContext::setMediaPosition(qint64 pos) {
	if(_player) {
		_player->setPosition(pos);
		emit onSeeked(pos);
	}
}

qint64 MediaPlayerContext::mediaDuration() {
	return _player ? _player->duration() : -1;
}

bool MediaPlayerContext::isSeekable() {
	return _player->isSeekable();
}

MediaContentPlayer * MediaPlayerContext::player() {
	if(!_player) {
		_player = new MediaContentPlayer(this);
		connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(onStateChanged(QMediaPlayer::State)));
		//connect(_player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(onMediaChanged(QMediaContent)));

		connect(_player, SIGNAL(videoAvailableChanged(bool)), this, SIGNAL(videoAvailableChanged(bool)));
		
		connect(_player, SIGNAL(durationChanged(qint64)), this, SIGNAL(durationChanged(qint64)));
		connect(_player, SIGNAL(volumeChanged(int)), this, SIGNAL(volumeChanged(int)));
		
		connect(_player, SIGNAL(playbackRateChanged(qreal)), this, SIGNAL(playbackRateChanged(qreal)));

		connect(_player, SIGNAL(seekableChanged(bool)), this, SIGNAL(isSeekableChanged(bool)));
		
// 		connect(_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(onPlayerError(QMediaPlayer::Error)));
	}

	return _player;
}

// void MediaPlayerContext::onPlayerError(QMediaPlayer::Error e) {
// 	if(e == QMediaPlayer::ResourceError) {
// 		qint64 pos = _player->position();
// 		_player->stop();
// 		_player->setPosition(pos);
// 		_player->play();
// 	} else {
// 		qDebug() << "Error:" << e;
// 	}
// }

MediaInformationManager * MediaPlayerContext::mediaInformationManager() const {
	return _mediaInformationManager;
}

MediaPlayerContext::~MediaPlayerContext() {
	if(_player) delete _player;
#ifdef MP_SUPPORT_MPRIS
	if(this->mpris) delete this->mpris;
#endif
	delete _pluginManager;
	delete _mediaInformationManager;
	if(currentSubtitles) delete currentSubtitles;
}
