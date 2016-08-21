#include "Mpris.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QMetaClassInfo>
#include <QStringList>

#include <unistd.h>

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */

Mpris::Mpris(MediaPlayerContext * context, MainWindow * gui) : QObject(context) {
	QString name("org.mpris.MediaPlayer2.PapyrosMediaPlayer");

	bool success = QDBusConnection::sessionBus().registerService(name);

	// If the above failed, it's likely because we're not the first instance
	// and the name is already taken. In that event the MPRIS2 spec wants the
	// following:
	if (!success)
		success = QDBusConnection::sessionBus().registerService(name + ".instance" + QString::number(getpid()));

	if (success) {
		player = new MprisMediaPlayer(context, gui, this);
		controller = new MprisMediaPlayerController(context, gui, this);
		QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors);
	}
}

Mpris::~Mpris() {
	if (player) delete player;
	if (controller) delete controller;
}

void Mpris::signalPropertiesChange(const QObject* adaptor, const QVariantMap& properties)
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties", "PropertiesChanged" );

    QVariantList args;
    args << adaptor->metaObject()->classInfo(0).value();
    args << properties;
    args << QStringList();

    msg.setArguments(args);

    QDBusConnection::sessionBus().send(msg);
}
