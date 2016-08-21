#include "MediaContentPlayer.h"

#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoSurfaceFormat>

MediaContentPlayer::MediaContentPlayer(QObject* parent, Flags flags): QMediaPlayer(parent, flags) {

}

void MediaContentPlayer::setVideoSurface(QAbstractVideoSurface* surface) {
	//qDebug() << "Changing surface to" << surface;
//	if(surface) {
//		qDebug() << surface->surfaceFormat();
//		surface->start(QVideoSurfaceFormat(QSize(140, 100), QVideoFrame::Format_RGB24, QAbstractVideoBuffer::EGLImageHandle));
//		//QObject::connect(surface, SIGNAL(nativeResolutionChanged(QSize)), this, SLOT(bla(QSize)));
//	}
	_surface = surface;
	setVideoOutput(surface);
}

QAbstractVideoSurface* MediaContentPlayer::getVideoSurface() {
	//qDebug() << "Getting surface?";
	return _surface;
}
