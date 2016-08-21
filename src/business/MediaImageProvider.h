#ifndef MEDIAIMAGEPROVIDER_H
#define MEDIAIMAGEPROVIDER_H

#include <QImage>
#include <QImageReader>
#include <QQuickImageResponse>
#include <QThreadPool>

class MediaImageProvider;
class MediaImageResponse;

#include "repo/Library.h"
#include "business/MediaInformationManager.h"

//class ThumbnailResponse : public QQuickImageResponse, public QRunnable {
//	public:
//		ThumbnailResponse(Library * library, const QString & id, const QSize & requestedSize);

//		void run();
//		QQuickTextureFactory *textureFactory() const;

////		void cancel();
//		// TODO very nasty workaround, Qt is deleting instances of MediaImageResponse before
//		// "emit finished()" happens...
////		bool event(QEvent*e) {
////			if(!canDeleteLater && e->type() == QEvent::DeferredDelete) {
////				cancel();
////				QMutexLocker lock(&waitBeforeDeleting);
////				return false;
////			}

////			return QQuickImageResponse::event(e);
////		}

//		~ThumbnailResponse();
//	private:
//		Library * library;
////		MediaInformationManager * mediaInformationManager;

////		MediaInformationRequestOptions options;

//		QString id;
//		QSize requestedSize;
//		bool shouldDeleteTexture = true;
//		QQuickTextureFactory * texture = NULL;

//		QMutex waitBeforeDeleting;
//		bool canDeleteLater = false;
//};

class MediaImageResponse : public QQuickImageResponse, public QRunnable {
	public:
		MediaImageResponse(MediaImageProvider * provider, Library * library, MediaInformationManager * mediaInformationManager, const QString & id, const QSize & requestedSize);

		void run();
		bool loadMediaPropertyImage(const QString & type, int id);
		bool loadThumbnail(int mediaContentId);
		QQuickTextureFactory *textureFactory() const;
		
		void cancel();
		// TODO very nasty workaround, Qt is deleting instances of MediaImageResponse before
		// "emit finished()" happens...
		bool event(QEvent*e) {
			if(!canDeleteLater && e->type() == QEvent::DeferredDelete) {
				cancel();
				//QMutexLocker lock(&waitBeforeDeleting);
				return false;
			}

			return QQuickImageResponse::event(e);
		}

		~MediaImageResponse();
	private:
		MediaImageProvider * provider;
		Library * library;
		MediaInformationManager * mediaInformationManager;

		MediaInformationRequestOptions options;

		QString id;
		QSize requestedSize;
		bool shouldDeleteTexture = true;
		QQuickTextureFactory * texture = NULL;

		QMutex waitBeforeDeleting;
		bool canDeleteLater = false;
};

class MediaImageProvider : public QQuickAsyncImageProvider { //QQuickImageProvider {
	friend class MediaImageResponse;

	public:
		MediaImageProvider(MediaPlayerContext & context);

//		QImage requestImage(const QString & id, QSize * size, const QSize & requestedSize);
		QQuickImageResponse * requestImageResponse(const QString & id, const QSize & requestedSize);

	private:
		MediaPlayerContext & context;

		struct Waiter {
			QMutex * mutex;
			QWaitCondition * waitCondition;

			Waiter() : mutex(), waitCondition() { }
			~Waiter() { delete mutex; delete waitCondition; }
		};

		QMap<QString, struct Waiter> tasks;
		QMutex tasksMutex;

		bool hasTask(const QString & id);
		void beginTask(const QString & id);
		void finishTask(const QString & id);
};

#endif // MEDIAIMAGEPROVIDER_H
