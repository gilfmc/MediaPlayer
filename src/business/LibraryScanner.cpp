#include "LibraryScanner.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>

#include <QtCore/QDebug>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <taglib/tpropertymap.h>

LibraryScanner::LibraryScanner(MediaPlayerContext * context) :
	context(context),
	library(context->library()),
	librarySettings(context->settings()->librarySettings()) {
}

void LibraryScanner::start() {
	QSqlQuery q("SELECT value FROM metadata WHERE key='lastScan'");
	if(q.next()) {
		QDateTime lastScan = q.value(0).toDateTime();
		if(lastScan.isValid()) {
			if(std::abs(lastScan.toTime_t() - QDateTime::currentMSecsSinceEpoch()) > 900000) {
				// it's been over 15 minutes since the last scan (or the system's time was wrong)
				scan();
			}
		} else {
			scan();
		}
	} else {
		scan();
	}
}

void LibraryScanner::refresh() {
	scan();
}

void LibraryScanner::onRun() {
	if(!metadataLoader) {
		metadataLoader = new MediaMetadataLoader(context);
	}

	LibraryLookupPlacesList * places = librarySettings->lookupPlaces();
	const int len = places->length();
	for(int i = 0; i < len; i++) {
		if(canceled()) {
			qDebug() << "Scanning stopped by request.";
			return;
		}

		LibraryLookupPlace * place = places->operator[](i);
		QDir folder(place->path());
		if(folder.exists())
			scanFolder(folder);
		else
			qDebug() << place->path() << "does not exist";
	}
	
	//player->deleteLater();
	//player = NULL;
	delete places;
}

void LibraryScanner::scanFolder(QDir folder) {
	qDebug() << " Scanning" << folder.path() << "...";
	QDirIterator it(folder, QDirIterator::Subdirectories);
	MediaContentLibrary * medias = library->mediaContents();
	while (it.hasNext()) {
		if(canceled()) {
			qDebug() << "Scanning stopped by request.";
			return;
		}

		it.next();
		const QString name = it.fileName();
		if(name == "." || name == "..") continue;

		const QFileInfo & file = it.fileInfo();
		if(file.isDir()) {
			//scanFolder(QDir(file.absoluteFilePath()));
		} else {
			const QString uri = "file://" + file.absoluteFilePath();
			if(!medias->has(uri)) {
				medias->registerMedia(metadataLoader->load(uri), this);
			}
		}
	}
}

void LibraryScanner::scan() {
	qDebug() << "Scanning library...";
//	if(!player) {
//		player = new QMediaPlayer(this);
//		connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(loadMediaData(QMediaPlayer::MediaStatus)));
//	}
	//_cancel = false;
	//QThreadPool::globalInstance()->start(this);
	context->taskManager()->run(this);
}

//void LibraryScanner::loadMediaData(QMediaPlayer::MediaStatus status) {
//	if(status == QMediaPlayer::LoadedMedia) {
//		processMedia = true;
//		mediaPlayerWaiter.wakeAll();
//	} else if(status == QMediaPlayer::LoadingMedia) {
//	} else {
//		// it probably failed, let's skip to the next one
//		processMedia = false;
//		mediaPlayerWaiter.wakeAll();
//	}
//}

// to be used later
QByteArray LibraryScanner::fileChecksum(const QString & fileName) {
	QFile f(fileName);
	if (f.open(QFile::ReadOnly)) {
		 QCryptographicHash hash(QCryptographicHash::Sha1);
		 if (hash.addData(&f)) {
				 return hash.result();
		 }
	}
	return QByteArray();
}

QString LibraryScanner::name() {
	return "Library scanner";
}

//void LibraryScanner::cancel() {
//	_cancel = true;
//}

LibraryScanner::~LibraryScanner() {
	if(metadataLoader) delete metadataLoader;
}
