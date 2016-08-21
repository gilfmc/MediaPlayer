#include "MprisMediaPlayer.h"

#include <QApplication>

#include "Mpris.h"

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */

MprisMediaPlayer::MprisMediaPlayer(MediaPlayerContext * context, MainWindow * gui, QObject * parent) :
QDBusAbstractAdaptor(parent), gui(gui), context(context) {
//     connect(m_gui, SIGNAL(fullScreen(bool)), this, SLOT(emitFullscreenChange(bool)));
}

MprisMediaPlayer::~MprisMediaPlayer() {
}

bool MprisMediaPlayer::CanQuit() const {
	return gui;
}

void MprisMediaPlayer::Quit() const {
	gui->quit();
}

bool MprisMediaPlayer::CanRaise() const {
	return gui;
}

void MprisMediaPlayer::Raise() const {
	gui->raise();
}

bool MprisMediaPlayer::Fullscreen() const {
	return gui->isFullscreen();
}

void MprisMediaPlayer::setFullscreen(bool fullscreen) const {
	gui->setFullscreen(fullscreen);
}

void MprisMediaPlayer::emitFullscreenChange(bool fullscreen) const {
//     QVariantMap properties;
//     properties["Fullscreen"] = fullscreen;
//     Mpris2::signalPropertiesChange(this, properties);
}

bool MprisMediaPlayer::CanSetFullscreen() const {
	return gui;
}

bool MprisMediaPlayer::HasTrackList() const {
	return false;
}

QString MprisMediaPlayer::Identity() const {
	return QString("Papyros Media Player");
}

QString MprisMediaPlayer::DesktopEntry() const {
	return QString("PapyrosMediaPlayer");
}

QStringList MprisMediaPlayer::SupportedUriSchemes() const {
	//TODO: Implement me
	return QStringList();
}

QStringList MprisMediaPlayer::SupportedMimeTypes() const {
	//TODO: Implement me
	return QStringList();
}
