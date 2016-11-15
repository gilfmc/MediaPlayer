#include "MediaInformationManager.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

MediaInformationManager::MediaInformationManager(MediaPlayerContext* context) : QObject(context), context(context)
{

}

void MediaInformationManager::getInformation(MediaProperty * property, QMap<QString, QVariant> & info, MediaInformationRequestOptions & options) {
	if(options.refresh == MediaInformationRequestOptions::ForceRefresh) {
		getExternalInformation(property, info, options);
		return;
	}

	const qint64 cTime = QDateTime::currentMSecsSinceEpoch();
	bool needsToBeRefreshed = false;
	if(options.cancel) return;
	QSqlQuery q;
	q.prepare("SELECT name, value, lastUpdate FROM mediaExtraInfo WHERE type=? AND propertyId=?");
	q.bindValue(0, property->type());
	q.bindValue(1, property->id());
	q.exec();
	QString fieldName;
	while (q.next()) {
		if(options.cancel) return;
		fieldName = q.value(0).toString();
		if(fieldName != "lastUsed") {
			if(cTime - q.value(2).toLongLong() > 2592000000) {
				needsToBeRefreshed = true;
				break;
			}
			
			if(options.cancel) return;
			info[fieldName] = q.value(1);
		}
	}

	if(options.refresh != MediaInformationRequestOptions::DontRefresh && (needsToBeRefreshed || info.count() == 0)) {
		getExternalInformation(property, info, options);
	} else {
		// register that we just looked for this info
		// this will be later useful to delete info that hasn't been looked for for a long time
		q.prepare("UPDATE mediaExtraInfo SET lastUsed=? WHERE type=? AND propertyId=? AND name='lastUsed'");
		q.bindValue(0, cTime);
		q.bindValue(1, property->type());
		q.bindValue(2, property->id());
		q.exec();
	}
}

void MediaInformationManager::getExternalInformation(MediaProperty * property, QMap<QString, QVariant> & info, MediaInformationRequestOptions & options) {
	PluginManager * plugins = context->pluginManager();
	Plugin<MediaInformationSourcePlugin> * plugin;
	MediaInformationSourcePlugin * source;

	const QString type = property->type();
	QMap<QString, QVariant> known;
	known["name"] = property->name();
	if(type == "episode") {
		Episode * e = (Episode*) property;
		known["number"] = e->number();
		Season * s = e->season();
		if(s) {
			known["season"] = s->number();
			if(s->parent())
				known["tvShow"] = s->parent()->name();
		}
	} else if(type == "artist") {
		Artist * a = (Artist*) property;
		AlsoKnownAsElement * akas = a->alsoKnownAs();
		if(akas) {
			QStringList akasList;
			const int len = akas->size();
			if(len > 0) {
				for(int i = 0; i < len; i++) {
					akasList.append(akas->get(i));
				}
				known["aka"] = akasList;
			}
		}
	}

	const int len = plugins->informationSourceCount();
	for(int i = 0; i < len; i++) {
		plugin = plugins->informationSource(i);
		source = plugin->instance();
		if(options.cancel) return;
		if(source) source->getInformation(type, known, info);
	}
	
	if(options.cancel || property->id() == -1) return;
	const qint64 cTime = QDateTime::currentMSecsSinceEpoch();
	qsrand(cTime);
	QSqlDatabase::database().transaction();
	QSqlQuery q;
	q.prepare("DELETE FROM mediaExtraInfo WHERE type=? AND propertyId=?");
	q.bindValue(0, type);
	q.bindValue(1, property->id());
	q.exec();
	q.prepare("INSERT INTO mediaExtraInfo (type, propertyId, name, value, lastUpdate) VALUES (?, ?, ?, ?, ?)");
	q.bindValue(0, type);
	q.bindValue(1, property->id());
	q.bindValue(4, cTime);
	QNetworkAccessManager nam;
	QString key;
	if(options.cancel) { QSqlDatabase::database().rollback(); return; }
	for (auto i = info.cbegin(), end = info.cend(); i != end; ++i) {
		if(options.cancel) { QSqlDatabase::database().rollback(); return; }
		key = i.key();
		if(key == "name" || key == "date") continue;
		q.bindValue(2, key);

		if(key == "poster" || key == "picture") {
			QUrl url(i.value().toString());
			if(url.isValid()) {
				QNetworkReply * reply = nam.get(QNetworkRequest(url));
				QEventLoop eventLoop;
				QObject::connect(&nam, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
				eventLoop.exec();
				QDir dir(QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + "/mediaImages");
				if(!dir.exists()) dir.mkpath(".");

				QString fileName;
				for (int i = 0; i < 8; i++)
					fileName[i] = qrand() % 100 < 50 ? QChar('A' + (qrand() % ('Z' - 'A'))) : QChar('a' + (qrand() % ('z' - 'a')));
				fileName = type + fileName;

				QFile file(dir.filePath(fileName));
				file.open(QIODevice::WriteOnly);
				file.write(reply->readAll());
				file.close();
				reply->deleteLater();

				qDebug() << "Downloaded picture" << url << "for" << property->type() << property->name() << "to" << dir.filePath(fileName);

				q.bindValue(3, fileName);
				info[i.key()] = fileName;
			} else {
				q.bindValue(3, QVariant());
			}
		} else {
			q.bindValue(3, i.value());
		}
		q.exec();
	}
	q.bindValue(2, "lastUsed");
	q.bindValue(3, QVariant());
	q.exec();
	QSqlDatabase::database().commit();
}

void MediaInformationManager::getInformation(MediaProperty * property, QMap<QString, QVariant> & info) {
	MediaInformationRequestOptions options;
	getInformation(property, info, options);
}

void MediaInformationManager::getExternalInformation(MediaProperty * property, QMap<QString, QVariant> & info) {
	MediaInformationRequestOptions options;
	getExternalInformation(property, info, options);
}

bool MediaInformationManager::checkExistence(ElementToCheck * element) {
	PluginManager * plugins = context->pluginManager();
	Plugin<InformationDetectorPlugin> * plugin;
	InformationDetectorPlugin * info;
	BlockingOperation bo;

	const int len = plugins->informationDetectorCount();
	for(int i = 0; i < len; i++) {
		plugin = plugins->informationDetector(i);
		info = plugin->instance();
		if(info) {
			connect(info, SIGNAL(onInformationDetected(InformationDetectorPlugin*)), &bo, SLOT(finish()));
			connect(info, SIGNAL(onInformationNotDetected(InformationDetectorPlugin*)), &bo, SLOT(finish()));
			info->checkExistence(element);
			bo.block(20000);
			disconnect(info, SIGNAL(onInformationDetected(InformationDetectorPlugin*)), &bo, SLOT(finish()));
			disconnect(info, SIGNAL(onInformationNotDetected(InformationDetectorPlugin*)), &bo, SLOT(finish()));
			bo.restart();
			if(!element->realName.isEmpty()) return true;
		}
	}

	return false;
}

//void MediaInformationManager::findMoreInformation(MediaContentInformationContainer* media) {
//	this->media = media;
//	currentPluginIndex = 0;
//	iterate();
//}

//void MediaInformationManager::iterate() {
//	InformationDetectorPlugin * id;
//	Plugin<InformationDetectorPlugin> * p = context->pluginManager()->informationDetector(currentPluginIndex);
//	if(!p->isInstanciated()) {
//		id = p->instance();
//		connect(id, SIGNAL(onInformationDetected(InformationDetectorPlugin*)), this, SLOT(onInformationDetected(InformationDetectorPlugin*)));
//		connect(id, SIGNAL(onInformationNotDetected(InformationDetectorPlugin*)), this, SLOT(onInformationNotDetected(InformationDetectorPlugin*)));
//	} else {
//		id = p->instance();
//	}
	
//	id->detectInformation(media);
	
//	currentPluginIndex++;
//}

//void MediaInformationManager::onInformationDetected(InformationDetectorPlugin*) {
//	emit onInformationFound(media);
//	media = NULL;
//}

//void MediaInformationManager::onInformationNotDetected(InformationDetectorPlugin*) {
//	if(context->pluginManager()->informationDetectorCount() < currentPluginIndex) {
//		qDebug() << "Still trying to look for info about" << media->mediaName;
//		iterate();
//	} else {
//		qDebug() << "Couldn't find info about" << media->mediaName;
//	}
//}

QString MediaInformationManager::getImageUrl(QString baseUrl) {
	QDir dir(QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + "/mediaImages");
	return dir.filePath(baseUrl);
}
