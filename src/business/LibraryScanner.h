#ifndef LIBRARYSCANNER_H
#define LIBRARYSCANNER_H

#include <QtCore/QCryptographicHash>
#include <QtCore/QThreadPool>
#include <QtCore/QWaitCondition>

#include <QtMultimedia/QMediaPlayer>

#include "business/Task.h"
#include "Settings.h"
#include "MediaPlayerContext.h"
#include "MediaMetadataLoader.h"

class LibraryScanner : QObject, Task
{
	Q_OBJECT
	
	public:
		LibraryScanner(MediaPlayerContext * context);

		void start();
		void refresh();

		virtual void onRun();
		
		QString name() override;
//		bool isRunning();
//		void cancel();

		virtual ~LibraryScanner();

	private:
		QByteArray fileChecksum(const QString &fileName);

		void scan();
		void scanFolder(QDir folder);

		MediaPlayerContext * context;
		Library * library;
		LibrarySettings * librarySettings;

		MediaMetadataLoader * metadataLoader = NULL;
		
		//QMediaPlayer * player = NULL;
		
//		QMutex mutexRunning;
//		//QWaitCondition mediaPlayerWaiter;
//		//bool processMedia;
	
//		bool running = false;
//		bool _cancel = false;
	//private slots:
		//void loadMediaData(QMediaPlayer::MediaStatus);
};

#endif // LIBRARYSCANNER_H
