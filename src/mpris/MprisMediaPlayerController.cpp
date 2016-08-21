#include "MprisMediaPlayerController.h"

#include <QtCore/QCryptographicHash>
#include <QtMultimedia/QMediaPlayer>

#include "Mpris.h"
#include "data/Song.h"
#include "MediaPlayerContext.h"

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */

MprisMediaPlayerController::MprisMediaPlayerController(MediaPlayerContext * context, MainWindow * gui, QObject * parent)
	: context(context), gui(gui), QDBusAbstractAdaptor(parent) {
	connect(context, SIGNAL(onPlaybackStateChanged(QMediaPlayer::State)), this, SLOT(onPlaybackStateChanged(QMediaPlayer::State)));
	connect(context, SIGNAL(playbackRateChanged(qreal)), this, SLOT(onPlaybackRateChanged(double)));
	connect(context, SIGNAL(onSeeked(qint64)), this, SLOT(onSeeked(qint64)));
//     connect(m_core, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
//     connect(m_core, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));

//     connect(m_core, SIGNAL(mediaPlaying(QString,QString)), this, SLOT(currentSourceChanged()));
//     connect(m_core, SIGNAL(stateChanged(Core::State)), this, SLOT(stateUpdated()));
//     connect(m_core, SIGNAL(mediaInfoChanged()), this, SLOT(emitMetadataChange()));
//     connect(m_core, SIGNAL(volumeChanged(int)), this, SLOT(volumeChanged()));

	connect(context->playlist(), SIGNAL(currentIndexChanged(int)), this, SLOT(currentMediaChanged(int)));
	connect(context->playlist(), SIGNAL(currentMediaDataChanged(MediaContent*)), this, SLOT(currentMediaDataChanged(MediaContent*)));
}

MprisMediaPlayerController::~MprisMediaPlayerController() {
}

void MprisMediaPlayerController::currentMediaChanged(int) const {
	currentMediaDataChanged(NULL);
}

void MprisMediaPlayerController::currentMediaDataChanged(MediaContent*) const {
	QVariantMap change;
	change["Metadata"] = this->metadata();
	Mpris::signalPropertiesChange(this, change);
}

bool MprisMediaPlayerController::canGoNext() const {
	return true;
}

void MprisMediaPlayerController::Next() const {
	context->next();
}

bool MprisMediaPlayerController::canGoPrevious() const {
	return true;
}

void MprisMediaPlayerController::Previous() const {
	context->previous();
}

bool MprisMediaPlayerController::canPause() const {
	return true;
}

void MprisMediaPlayerController::Pause() const {
	context->pause();
}

void MprisMediaPlayerController::PlayPause() const {
	if (context->playbackState() == QMediaPlayer::PlayingState) context->pause();
	else context->play();
}

void MprisMediaPlayerController::Stop() const {
	context->stop();
}

bool MprisMediaPlayerController::canPlay() const {
	return true;
}

void MprisMediaPlayerController::Play() const {
	context->play();
}

void MprisMediaPlayerController::SetPosition(const QDBusObjectPath & TrackId, qlonglong position) const {
	context->setMediaPosition(position / 1000);
//     if (TrackId.path().toLocal8Bit() == makeTrackId(m_core->mdat.filename))
//         m_core->seek(static_cast<int>(Position / 1000000));
}

void MprisMediaPlayerController::OpenUri(QString uri) const {
	//  m_core->open(uri);
}

QString MprisMediaPlayerController::playbackStatus() const {
	return stateToString(context->playbackState());
}

QString MprisMediaPlayerController::loopStatus() const {
	return "None";
}

void MprisMediaPlayerController::setLoopStatus(const QString & loopStatus) const {
	Q_UNUSED(loopStatus)
}

double MprisMediaPlayerController::rate() const {
	return context->playbackRate();
}

void MprisMediaPlayerController::setRate(double rate) const {
	context->setPlaybackRate(rate);
}

void MprisMediaPlayerController::onPlaybackRateChanged(double rate) const {
	QVariantMap change;
	change["Rate"] = this->rate();
	Mpris::signalPropertiesChange(this, change);
}

bool MprisMediaPlayerController::shuffle() const {
	return false;
}

void MprisMediaPlayerController::setShuffle(bool shuffle) const {
	Q_UNUSED(shuffle)
}

QVariantMap MprisMediaPlayerController::metadata() const {
	QVariantMap metaData;

//     if (!m_core->mdat.initialized)
//         return metaData;
	MediaContent * media = context->playlist()->currentMedia();
	Song * song = media && media->type() == "song" ? (Song*) media : NULL;

	metaData["mpris:length"] = context->mediaDuration() * 1000; // m_core->mdat.duration * 1000000;

//     if (m_core->mdat.type == TYPE_STREAM)
//         metaData["xesam:url"] = "";// m_core->mdat.stream_url;
//     else
	if(media) metaData["xesam:url"] = media->uri().toString(); //m_core->mdat.filename;

//     if (!m_core->mdat.clip_name.isEmpty())
	if(media) metaData["xesam:title"] = media->name(); // "title here"; //m_core->mdat.clip_name;
//     else if (!m_core->mdat.filename.isEmpty()) {
//         QFileInfo fileInfo(m_core->mdat.filename);
//         metaData["xesam:title"] = ""; // fileInfo.fileName();
//     }
	if (song) {
	metaData["mpris:trackid"] = 0; // QVariant::fromValue<QDBusObjectPath>(QDBusObjectPath(makeTrackId(m_core->mdat.filename).constData()));
//     if (!m_core->mdat.clip_album.isEmpty())
	metaData["xesam:album"] = "alBUm"; //m_core->mdat.clip_album;
//     if (!m_core->mdat.clip_artist.isEmpty())
		Artist * artist = song->artist();
		if (artist) metaData["xesam:artist"] = artist->name(); // m_core->mdat.clip_artist;
//     if (!m_core->mdat.clip_genre.isEmpty())
		metaData["xesam:genre"] = "genre"; // m_core->mdat.clip_genre;
	}

	return metaData;
}

double MprisMediaPlayerController::volume() const {
	return static_cast<double>(context->volume() / 100.0);
}

void MprisMediaPlayerController::setVolume(double volume) const {
	context->setVolume(volume * 100);
}

qlonglong MprisMediaPlayerController::position() const {
	return context->mediaPosition() * 1000; //static_cast<qlonglong>(m_core->mset.current_sec * 1000000);
}

void MprisMediaPlayerController::onSeeked(qint64 pos) const {
	emit Seeked(pos * 1000);
//	QVariantMap change;
//	change["Position"] = position();
//	Mpris::signalPropertiesChange(this, change);
}

double MprisMediaPlayerController::minimumRate() const {
	return 0.01;
}

double MprisMediaPlayerController::maximumRate() const {
	return 100.0;
}

bool MprisMediaPlayerController::canSeek() const {
	return true;
}

void MprisMediaPlayerController::Seek(qlonglong offset) const {
	context->setMediaPosition(context->mediaPosition() + offset / 1000);
}

bool MprisMediaPlayerController::canControl() const {
	return true;
}

//void MprisMediaPlayerController::tick(qint64 newPos) {
////     if (newPos - oldPos > tickInterval + 250 || newPos < oldPos)
////         emit Seeked(newPos * 1000);

//	// oldPos = newPos;
//}

void MprisMediaPlayerController::emitMetadataChange() const {
//     QVariantMap properties;
//     properties["Metadata"] = Metadata();
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::currentSourceChanged() const {
//     QVariantMap properties;
//     properties["Metadata"] = Metadata();
//     properties["CanSeek"] = CanSeek();
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::stateUpdated() const {
//     QVariantMap properties;
//     properties["PlaybackStatus"] = PlaybackStatus();
//     properties["CanPause"] = CanPause();
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::totalTimeChanged() const {
//     QVariantMap properties;
//     properties["Metadata"] = Metadata();
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::seekableChanged(bool seekable) const {
//     QVariantMap properties;
//     properties["CanSeek"] = seekable;
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::volumeChanged() const {
//     QVariantMap properties;
//     properties["Volume"] = Volume();
//     Mpris2::signalPropertiesChange(this, properties);
}

void MprisMediaPlayerController::onPlaybackStateChanged(QMediaPlayer::State state) const {
	//emit playbackStatusChanged(stateToString(state));
	QVariantMap change;
	change["PlaybackStatus"] = playbackStatus();
	Mpris::signalPropertiesChange(this, change);
}

QString MprisMediaPlayerController::stateToString(QMediaPlayer::State state) {
	switch (state) {
		case QMediaPlayer::PlayingState:
			return "Playing";
		case QMediaPlayer::PausedState:
			return "Paused";
		case QMediaPlayer::StoppedState:
			return "Stopped";
	}
	return "";
}
