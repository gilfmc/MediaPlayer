#ifndef MPRISMEDIAPLAYER_H
#define MPRISMEDIAPLAYER_H

class MprisMediaPlayer;

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtCore/QStringList>

#include "MediaPlayerContext.h"
#include "ui/MainWindow.h"

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */
class MprisMediaPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2") // Docs: http://www.mpris.org/2.1/spec/Root_Node.html

    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool CanRaise READ CanRaise)

    Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)

    Q_PROPERTY(bool HasTrackList READ HasTrackList)

    Q_PROPERTY(QString Identity READ Identity)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry)

    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)

    public:
		 explicit MprisMediaPlayer(MediaPlayerContext * context, MainWindow * gui, QObject * parent);
		  ~MprisMediaPlayer();

        bool CanQuit() const;
        bool CanRaise() const;

        bool Fullscreen() const;
        void setFullscreen(bool fullscreen) const;
        bool CanSetFullscreen() const;

        bool HasTrackList() const;

        QString Identity() const;
        QString DesktopEntry() const;

        QStringList SupportedUriSchemes() const;
        QStringList SupportedMimeTypes() const;

    public slots:
        void Raise() const;
        void Quit() const;

    private slots:
        void emitFullscreenChange(bool fullscreen) const;
		  
    private:
        MediaPlayerContext * context;
		MainWindow * gui;
};

#endif //MPRISMEDIAPLAYER_H
