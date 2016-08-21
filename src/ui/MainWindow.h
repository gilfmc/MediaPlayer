#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class MainWindow;

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuick/QQuickWindow>

#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>

#include <QtQuick/QQuickItem>

#include "MediaPlayerContext.h"
#include "repo/Library.h"
#include "MediaContentUiProvider.h"
#include "ui/PropertyListUiProvider.h"
#include "business/Subtitles.h"

class LibraryModel : public QAbstractListModel {
	Q_OBJECT
	
	public:
		LibraryModel(MediaPlayerContext &context, QObject * parent);
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;
	
		Q_INVOKABLE
		MediaContentUiProvider * getItem(int index) const;

		Q_INVOKABLE
		void playAll(int index);
		Q_INVOKABLE
		void playAll(const QString & type, int id);
		void playAll(MediaProperty *);

		Q_INVOKABLE
		void removeFromLibrary(int index);
		Q_INVOKABLE
		void removeFromLibrary(const QString & type, int id);
		void removeFromLibrary(MediaProperty *);

		Q_INVOKABLE
		void addToLibrary(const QString & type, int id);
		void addToLibrary(MediaProperty *);

		Q_INVOKABLE
		void renameMediaProperty(const QString & type, int id, const QString & newName);

		Q_INVOKABLE
		void undoRemoval();

		Q_INVOKABLE
		QString getMediaPropertyName(const QString & type, int id);

		Q_INVOKABLE
		QString getSingularItemName(int index);
		Q_INVOKABLE
		QString getInternalGroupName(int index);
		Q_INVOKABLE
		MediaContentUiProvider * registerItem(QString group, QString name);
		Q_INVOKABLE
		PropertyListUiProvider * getListUiAt(int groupAt);
		Q_INVOKABLE
		PropertyListUiProvider * getListUi(int index);

	private slots:
		void onMediaPropertyAddedToLibrary(const QString &, MediaProperty *);
		void onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *);
	
	private:
		MediaPlayerContext * context;
		Library * library;

		int getGroupIndex(int at);

		typedef struct LibraryListItem {
				char type;
				QString name;
				QString image;
				MediaProperty * media;

				LibraryListItem(QString name, QString image) : type(0), name(name), image(image), media(NULL) {}
				LibraryListItem(MediaProperty * media) : type(1), name(media->name()), image(QString::null), media(media) {}
		} LibraryListItem;
		QList<const LibraryListItem*> items;

		MediaProperty * lastRemovedItem = NULL;
};

class MainWindow : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool subtitlesEnabled READ subtitlesEnabled WRITE setSubtitlesEnabled) // NOTIFY subtitlesEnabledChanged)
	Q_PROPERTY(int subtitleDelay READ subtitleDelay WRITE setSubtitleDelay) // NOTIFY subtitleDelayChanged)
	
	public:
		explicit MainWindow(QQmlApplicationEngine &engine, MediaPlayerContext &context);
		~MainWindow();

		bool subtitlesEnabled();
		void setSubtitlesEnabled(bool enabled);
		int subtitleDelay();
		void setSubtitleDelay(int delay);
		void loadSubtitlesFromFile(QUrl file);

		Q_INVOKABLE
		MediaContentUiProvider * getMediaContentUiProvider(const QString & type, int id);

	private:
		QQmlApplicationEngine &engine;
		MediaPlayerContext &context;

		QMediaPlayer * player();

		QQuickWindow * uiRoot;
		
		QQuickItem * lLength, * lTime, * bMediaProgress, * lSubtitle;
		bool hasHour;
		
		QTimer * timer = NULL;
		//int toW, toH;
		//virtual void timerEvent(QTimerEvent*);
		
		QTimer errorRecoveryTimer;

		QTimer subtitleTimer;
		bool _subtitlesEnabled = true;
		int _subtitleDelay = 0;
// 		int errorThrownAt;
// 		State errorThrownState;
		
	signals:

	public slots:
		//void onPlayClicked();
		void onMediaOpened(QVariant);

		void onVolumeChanged(int);

		void setCursorVisible(bool);

//		void onPreviousPressed();
//		void onNextPressed();
		
		bool isFullscreen();
		void setFullscreen(bool fullscreen);
		
		void raise();
		void quit();
		
	private slots:
		void durationChanged(qint64 duration);
		void onPlaybackStateChanged(QMediaPlayer::State state);
		void mediaTick();
		void playbackRateChanged(qreal);
		void onSeeked(qint64);

		void subtitleTimerTimeout();
		void onHasSubtitlesChanged(bool, Subtitles*);
		
// 		void currentIndexChanged(int);
		
		void errorRecoveryTimerTimeout();
		void onError(QMediaPlayer::Error);
		
//		void potatoTest(QString test);
};

#endif // MAINWINDOW_H
