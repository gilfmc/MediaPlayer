#ifndef MPRISMEDIAPLAYERCONTROLLER_H
#define MPRISMEDIAPLAYERCONTROLLER_H

class MprisMediaPlayerController;

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusObjectPath>

#include "MediaPlayerContext.h"
#include "data/MediaContent.h"
#include "ui/MainWindow.h"

/**
 * Adapter from SMPlayer (http://smplayer.sourceforge.net/)
 */
class MprisMediaPlayerController : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player") // Docs: http://www.mpris.org/2.1/spec/Player_Node.html

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)

    public:
        explicit MprisMediaPlayerController(MediaPlayerContext * context, MainWindow * gui, QObject * parent);
        ~MprisMediaPlayerController();

        QString playbackStatus() const;
        QString loopStatus() const;
        void setLoopStatus(const QString& loopStatus) const;
        double rate() const;
        void setRate(double rate) const;
        bool shuffle() const;
        void setShuffle(bool shuffle) const;
        QVariantMap metadata() const;
        double volume() const;
        void setVolume(double volume) const;
        qlonglong position() const;
        double minimumRate() const;
        double maximumRate() const;
        bool canGoNext() const;
        bool canGoPrevious() const;
        bool canPlay() const;
        bool canPause() const;
        bool canSeek() const;
        bool canControl() const;

    signals:
		//void playbackStateChanged(QString state) const;
        void Seeked(qlonglong Position) const;

    public slots:
        void Next() const;
        void Previous() const;
		
        void Pause() const;
        void PlayPause() const;
        void Stop() const;
        void Play() const;
		
        void Seek(qlonglong Offset) const;
        void SetPosition(const QDBusObjectPath& TrackId, qlonglong Position) const;
        void OpenUri(QString uri) const;

    private slots:
				//void tick(qint64 newPos);
        void emitMetadataChange() const;
        void currentSourceChanged() const;
        void stateUpdated() const;
        void totalTimeChanged() const;
        void seekableChanged(bool seekable) const;
        void volumeChanged() const;

		void onPlaybackStateChanged(QMediaPlayer::State) const;
		void onPlaybackRateChanged(double rate) const;
		void onSeeked(qint64 pos) const;
		
		void currentMediaChanged(int) const;
        void currentMediaDataChanged(MediaContent*) const;

    private:
        //qint64 oldPos;
        //Core* m_core;
        //Playlist* m_playlist;
		 MediaPlayerContext * context;
		 MainWindow * gui;
		 
		 static QString stateToString(QMediaPlayer::State);
};

#endif //MPRISMEDIAPLAYERCONTROLLER_H
