#ifndef MEDIAPLAYERCONTEXT_H
#define MEDIAPLAYERCONTEXT_H

#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>

//#include <MediaInfo/MediaInfo.h>
//#include <MediaInfo/MediaInfo_Config.h>
//#include <MediaInfo/Setup.h>

class MediaPlayerContext;

#include "business/PluginManager.h"
#include "business/MediaInformationManager.h"
#include "business/Settings.h"
#include "business/TaskManager.h"
#include "ui/MainWindow.h"
#include "repo/Library.h"
#include "data/Playlist.h"
#include "MediaContentPlayer.h"

#include "business/Subtitles.h"

#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
#define MP_SUPPORT_DBUS
#define MP_SUPPORT_FFMPEGTHUMBNAILER
#endif
#endif

#ifdef MP_SUPPORT_DBUS
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#define MP_SUPPORT_MPRIS
#endif

#ifdef MP_SUPPORT_MPRIS
#include "mpris/Mpris.h"
#endif

class SubtitlesAsyncLoader : public Task {
	public:
		SubtitlesAsyncLoader(MediaContent * content) : content(content), fromUrl(false) { }
		SubtitlesAsyncLoader(Playlist * playlist) : playlist(playlist), fromUrl(false) { }
		SubtitlesAsyncLoader(MediaContent * content, QUrl url) : content(content), fromUrl(true), url(url) { }
		virtual QString name() { return "Subtitle loader"; }

		virtual void onRun();

		MediaContent * media() { return content; }

		bool hasSubtitles() { return _subtitles; }
		Subtitles * subtitles() { return _subtitles; }

	private:
		MediaContent * content = NULL;
		Playlist * playlist = NULL;
		bool fromUrl;
		QUrl url;
		Subtitles * _subtitles = NULL;
};

class MediaPlayerContext : public QObject {
		Q_OBJECT
		//Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
		Q_PROPERTY(QMediaPlayer::State playbackState READ playbackState NOTIFY onPlaybackStateChanged)
		Q_PROPERTY(bool hasMediaToPlay READ hasMediaToPlay)
		Q_PROPERTY(qint64 mediaPosition READ mediaPosition)
		Q_PROPERTY(qint64 mediaDuration READ mediaDuration)
		Q_PROPERTY(bool isSeekable READ isSeekable NOTIFY isSeekableChanged)
		Q_PROPERTY(qreal playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
		Q_PROPERTY(LoopMode loopMode READ loopMode WRITE setLoopMode NOTIFY loopModeChanged)

		Q_PROPERTY(bool videoAvailable READ isVideoAvailable NOTIFY videoAvailableChanged)
		
		Q_PROPERTY(PluginManager * pluginManager READ pluginManager)
		Q_PROPERTY(Library * library READ library)
		Q_PROPERTY(Playlist * playlist READ playlist NOTIFY onPlaylistChanged)
		Q_PROPERTY(Settings * settings READ settings)

		Q_ENUMS(LoopMode)

	public:
		enum LoopMode { CurrentItemInLoop, Sequential, Loop };

		explicit MediaPlayerContext(QObject *parent = 0);
		
		void startMpris(MainWindow * gui = 0);

		MediaContentPlayer * player();
		Playlist * playlist();
		Library * library();

		int volume();
		
		QMediaPlayer::State playbackState();
		//bool isPlaying();
		bool hasMediaToPlay();
		qint64 mediaPosition();
		qint64 mediaDuration();
		bool isSeekable();
		LoopMode loopMode();
		
		qreal playbackRate();

		bool isVideoAvailable();

		void loadSubtitles(QUrl url);
		bool hasSubstitles();
		Subtitles * subtitles();

		TaskManager * taskManager();

		PluginManager * pluginManager();

		MediaInformationManager * mediaInformationManager() const;
		
		Settings * settings();

		QString formatLength(MediaContent *) const;
		QString formatLength(qint64 len) const;

		Q_INVOKABLE
		void refreshLibrary();
		Q_INVOKABLE
		bool isLibraryBeingRefreshed();

		~MediaPlayerContext();

	private:
		MediaContentPlayer * _player = NULL;
		#ifdef MP_SUPPORT_MPRIS
		Mpris * mpris;
		#endif
        Settings * _settings;
		PluginManager * _pluginManager;
		MediaInformationManager * _mediaInformationManager;
		Playlist * _playlist = NULL;
        Library * _library = NULL;
		TaskManager _taskManager;
		Subtitles * currentSubtitles = NULL;

		void emitSubtitlesChange();

		class OnSubtitlesLoaded : public TaskOwner {
			public:
				OnSubtitlesLoaded(MediaPlayerContext & context) : context(context) {}
				virtual void onFinishedRunning(Task * task);
			private:
				MediaPlayerContext & context;
		};

	signals:
		//void isPlayingChanged(bool isPlaying);
		void durationChanged(qint64 duration);
		void volumeChanged(int volume);
		void videoAvailableChanged(bool videoAvailable);
		void onPlaybackStateChanged(QMediaPlayer::State);
		void playbackRateChanged(qreal);
		void isSeekableChanged(bool isSeekable);
		void onSeeked(qint64 position);
		void loopModeChanged(LoopMode mode);
		
		void onPlaylistChanged();

		void onHasSubtitlesChanged(bool has, Subtitles * subtitles);

	public slots:
		void onStateChanged(QMediaPlayer::State);
		void onMediaChanged(QMediaContent);
		
		void setVolume(int);

		void play();
		void pause();
		void stop();
		
		void previous();
		void next();
		
		void setMediaPosition(qint64);

		void setLoopMode(LoopMode mode);
		
		void setPlaybackRate(qreal rate);
	
	private slots:
//		void onInformationFound(MediaContentInformationContainer * media);
		void currentPlaylistIndexChanged(int);
// 		void onPlayerError(QMediaPlayer::Error);
};

#endif // MEDIAPLAYERCONTEXT_H
