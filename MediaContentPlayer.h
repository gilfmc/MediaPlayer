#ifndef MEDIACONTENTPLAYER_H
#define MEDIACONTENTPLAYER_H

#include <QtMultimedia/QMediaPlayer>

#include <QtCore/QSize>
#include <QtCore/QTimer>

class MediaContentPlayer : public QMediaPlayer
{
		Q_OBJECT
		Q_PROPERTY(QAbstractVideoSurface * videoSurface READ getVideoSurface WRITE setVideoSurface)
	public:
		MediaContentPlayer(QObject * parent = 0, Flags flags = 0);

	public slots:
		void setVideoSurface(QAbstractVideoSurface * surface);
		QAbstractVideoSurface * getVideoSurface();

	private:
		QAbstractVideoSurface * _surface;
		
	private slots:
};

#endif // MEDIACONTENTPLAYER_H
