#ifndef MEDIAINFORMATIONMANAGER_H
#define MEDIAINFORMATIONMANAGER_H

#include <QtCore/QWaitCondition>

class MediaInformationManager;

#include "MediaPlayerContext.h"
#include "data/MediaProperty.h"
#include "business/InformationDetectorPlugin.h"

class BlockingOperation : public QObject {
		Q_OBJECT
	public:
		void block() {
			if(finished) return;

			mutex.lock();
			waiter.wait(&mutex);
			mutex.unlock();
		}

		void block(unsigned long wait) {
			if(finished) return;

			mutex.lock();
			waiter.wait(&mutex, wait);
			mutex.unlock();
		}

		void restart() {
			finished = false;
		}

	public slots:
		void finish() {
			finished = true;
			waiter.wakeAll();
		}

	private:
		QMutex mutex;
		QWaitCondition waiter;
		bool finished = false;
};

struct MediaInformationRequestOptions {
	enum RefreshOptions { RefreshIfNeeded, ForceRefresh, DontRefresh };

	RefreshOptions refresh = RefreshIfNeeded;
	/**
	 * @brief onlyLookingForImagery set to true if you only need images,
	 * this doesn't guarantee it will only get images if it's not
	 * significantly cheaper getting just images
	 */
	bool onlyLookingForImagery = false;
	
	/**
	 * For concurrence reasons, set to true to cancel loading
	 */
	bool cancel = false;
};

class MediaInformationManager : public QObject {
	Q_OBJECT

	public:
		MediaInformationManager(MediaPlayerContext * context);

		//void findMoreInformation(MediaContentInformationContainer * media);

		bool checkExistence(ElementToCheck * element);

		void getInformation(MediaProperty * property, QMap<QString, QVariant> & info);
		void getInformation(MediaProperty * property, QMap<QString, QVariant> & info, MediaInformationRequestOptions & options);
		void getExternalInformation(MediaProperty *property, QMap<QString, QVariant> & info);
		void getExternalInformation(MediaProperty *property, QMap<QString, QVariant> & info, MediaInformationRequestOptions & options);

		QString getImageUrl(QString baseUrl);

	//signals:
		//void onInformationFound(MediaContentInformationContainer * media);
		
	private:
		MediaPlayerContext * context;
		
		//MediaContentInformationContainer * media;
		//int currentPluginIndex;
		//void iterate();

//	private slots:
//		void onInformationDetected(InformationDetectorPlugin*);
//		void onInformationNotDetected(InformationDetectorPlugin*);
};

#endif // MEDIAINFORMATIONMANAGER_H
