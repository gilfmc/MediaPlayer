#ifndef MPRIS_H
#define MPRIS_H

class Mpris;

#include <QtCore/QObject>
#include <QVariantMap>

#include "MediaPlayerContext.h"
#include "ui/MainWindow.h"

#include "MprisMediaPlayer.h"
#include "MprisMediaPlayerController.h"

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */
class Mpris : public QObject {
	Q_OBJECT

public:
	explicit Mpris(MediaPlayerContext * context, MainWindow * gui = 0);
	~Mpris();

static void signalPropertiesChange(const QObject* adaptor, const QVariantMap& properties);

private:
	MprisMediaPlayer * player;
	MprisMediaPlayerController * controller;
};

#endif //MPRIS_H
