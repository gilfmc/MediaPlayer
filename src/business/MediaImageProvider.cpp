#include "MediaImageProvider.h"

#ifdef MP_SUPPORT_FFMPEGTHUMBNAILER
#include "libffmpegthumbnailer/videothumbnailer.h"
#endif

MediaImageProvider::MediaImageProvider(MediaPlayerContext &context) : /*QQuickImageProvider(QQuickImageProvider::Image),*/ context(context) { }

#define MIR_FINISH provider->finishTask(this->id); canDeleteLater = true; emit finished();// deleteLater();
#define MIR_STOPIFCANCELED if(options.cancel) { MIR_FINISH return false; }

bool MediaImageProvider::hasTask(const QString & id) {
	QMutexLocker locker(&tasksMutex);
	if(tasks.contains(id)) {
		tasks[id].waitCondition->wait(tasks[id].mutex);
		return true;
	} else {
		return false;
	}
}

void MediaImageProvider::beginTask(const QString & id) {
	QMutexLocker locker(&tasksMutex);
	if(tasks.contains(id))
		tasks[id].waitCondition->wait(tasks[id].mutex);
	else tasks[id];
}

void MediaImageProvider::finishTask(const QString & id) {
	QMutexLocker locker(&tasksMutex);
	tasks.remove(id);
}

QQuickImageResponse * MediaImageProvider::requestImageResponse(const QString & id, const QSize & requestedSize) {
//	if(id.startsWith("thumb")) {
//		ThumbnailResponse * response = new ThumbnailResponse(context.library(), id, requestedSize);
//		QThreadPool::globalInstance()->start(response);
//		return response;
//	} else {
		MediaImageResponse * response = new MediaImageResponse(this, context.library(), context.mediaInformationManager(), id, requestedSize);
		QThreadPool::globalInstance()->start(response);
		return response;
//	}
}

MediaImageResponse::MediaImageResponse(MediaImageProvider * provider, Library * library, MediaInformationManager * mediaInformationManager, const QString & id, const QSize & requestedSize) : provider(provider), library(library), mediaInformationManager(mediaInformationManager), id(id), requestedSize(requestedSize) {
	setAutoDelete(false);
}

void MediaImageResponse::cancel() {
	options.cancel = true;
}

void MediaImageResponse::run() {
	if(provider->hasTask(id)) {
		qDebug() << "Someone asked for the same image (" << id << ") more than once at the same time...";
	}
	provider->beginTask(id);
	QMutexLocker lock(&waitBeforeDeleting);
	QStringList path = id.split("/");
	if(path.length() > 1) {
		if(path[0].startsWith("thumb")) {
			if(loadThumbnail(path[1].toInt())) {
				MIR_FINISH
			}
		} else {
			if(loadMediaPropertyImage(path[0], path[1].toInt())) {
				MIR_FINISH
			}
		}
	}

	MIR_FINISH
}

bool MediaImageResponse::loadMediaPropertyImage(const QString & type, int id) {
	MediaInformationManager * mediaInformationManager = this->mediaInformationManager;

	if(type == "tvShow") {
		TvShow * tvShow = library->tvShows()->get(id);
		if(tvShow) {
			QMap<QString, QVariant> info;
			options.onlyLookingForImagery = true;
			mediaInformationManager->getInformation(tvShow, info, options);

			MIR_STOPIFCANCELED

			QImageReader imager;
			QImage image;

			bool shouldResize = requestedSize.isValid();
			if(shouldResize) {
				QSize finalSize = imager.size();
				if(finalSize.isValid()) {
					finalSize.scale(requestedSize, Qt::KeepAspectRatioByExpanding);
					imager.setScaledSize(finalSize);
					shouldResize = false;
				}
			}

			MIR_STOPIFCANCELED

			if(info["poster"].toString().length() > 0) {
				imager.setFileName(mediaInformationManager->getImageUrl(info["poster"].toString()));
				imager.read(&image);

				if(image.isNull()) {
					options.refresh = MediaInformationRequestOptions::ForceRefresh;
					mediaInformationManager->getInformation(tvShow, info, options);
					imager.setFileName(mediaInformationManager->getImageUrl(info["poster"].toString()));
					imager.read(&image);

					MIR_STOPIFCANCELED
				}
				if(!image.isNull()) {
//						if(shouldResize)
//							image = image.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

					//*size = QSize(image.width(), image.height());

					texture = QQuickTextureFactory::textureFactoryForImage(image);
				} else {
					qDebug() << "Error reading" << this->id << imager.fileName() << ":" << imager.error();
				}
			} else {
				image = QImage(1, 1, QImage::Format_ARGB32);
				image.setPixel(0, 0, 0xffffffff);
				texture = QQuickTextureFactory::textureFactoryForImage(image);
			}
		}
	} else if(type == "artist") {
		Artist * a = library->artists()->get(id);
		if(a) {
			QMap<QString, QVariant> info;
			//options.refresh = MediaInformationRequestOptions::DontRefresh;
			options.onlyLookingForImagery = true;
			mediaInformationManager->getInformation(a, info, options);

			MIR_STOPIFCANCELED

			QImageReader imager;
			QImage image;

			bool shouldResize = requestedSize.isValid();
			if(shouldResize) {
				QSize finalSize = imager.size();
				if(finalSize.isValid()) {
					finalSize.scale(requestedSize, Qt::KeepAspectRatioByExpanding);
					imager.setScaledSize(finalSize);
					shouldResize = false;
				}
			}

			MIR_STOPIFCANCELED

			if(info["picture"].toString().length() > 0) {
				imager.setFileName(mediaInformationManager->getImageUrl(info["picture"].toString()));
				imager.read(&image);

				if(image.isNull()) {
					options.refresh = MediaInformationRequestOptions::ForceRefresh;
					mediaInformationManager->getInformation(a, info, options);
					imager.setFileName(mediaInformationManager->getImageUrl(info["picture"].toString()));
					imager.read(&image);

					MIR_STOPIFCANCELED
				}
				if(!image.isNull()) {
//						if(shouldResize)
//							image = image.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

					//*size = QSize(image.width(), image.height());

					texture = QQuickTextureFactory::textureFactoryForImage(image);
				} else {
					qDebug() << "Error reading" << this->id << imager.fileName() << ":" << imager.error();
				}
			} else {
				image = QImage(1, 1, QImage::Format_ARGB32);
				image.setPixel(0, 0, 0xffffffff);
				texture = QQuickTextureFactory::textureFactoryForImage(image);
			}
		}
	}

	return true;
}

QQuickTextureFactory * MediaImageResponse::textureFactory() const {
	//shouldDeleteTexture = false;
	return texture;
}

MediaImageResponse::~MediaImageResponse() {
	cancel();
	QMutexLocker lock(&waitBeforeDeleting);
	//if(shouldDeleteTexture && texture) delete texture;
}

//ThumbnailResponse::ThumbnailResponse(Library * library, const QString & id, const QSize & requestedSize) : library(library), id(id), requestedSize(requestedSize) {
//	setAutoDelete(false);
//}

//void ThumbnailResponse::run() {
bool MediaImageResponse::loadThumbnail(int mediaContentId) {
	Library * library = this->library;
	//QSize requestSize = this->requestedSize;

	MediaContent * media = library->mediaContents()->get(mediaContentId);
	if(media) {
		QDir thumbsFolder(QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0]);
		thumbsFolder.mkdir("thumbnails");
		thumbsFolder.cd("thumbnails");
		QString thumbPath = thumbsFolder.filePath(QString::number(mediaContentId));
		QImage image(thumbPath);
		if(image.isNull()) {
			try {
#ifdef MP_SUPPORT_FFMPEGTHUMBNAILER
				qDebug() << "Generating new thumbnail for" << media->type() << media->name() << "(" << media->uri() << ")...";
				ffmpegthumbnailer::VideoThumbnailer thumbnailer(360, false, true, 60, true);
				thumbnailer.generateThumbnail(media->uri().path().toStdString(), Jpeg, thumbPath.toStdString());
				image = QImage(thumbPath);
#endif
			} catch (...) {
				qDebug() << "Failed to load thumbnail for" << media->type() << media->name() << "(" << media->uri() << ")";
			}
		}
		if(!image.isNull()) {
			if(requestedSize.isValid()) image = image.scaled(requestedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
			texture = QQuickTextureFactory::textureFactoryForImage(image);
		}
	}

	delete media;

	//canDeleteLater = true;
	//emit finished();
	//deleteLater();

	return true;
}

//QQuickTextureFactory * ThumbnailResponse::textureFactory() const {
//	return texture;
//}

//ThumbnailResponse::~ThumbnailResponse() {
//	cancel();
//	QMutexLocker lock(&waitBeforeDeleting);
//}
