#include "DiscogsInformationDetectorPlugin.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

DiscogsInformationDetectorPlugin::DiscogsInformationDetectorPlugin()
{
}

void DiscogsInformationDetectorPlugin::detectInformation(MediaContentInformationContainer * container) {
	currentContainer = container;
	contactServer("database/search", "q=" + QUrl::toPercentEncoding(container->mediaName) + "&type=release&per_page=5");
}

bool DiscogsInformationDetectorPlugin::supports(QString mediaContentType) {
	return mediaContentType == "audio" || mediaContentType == "video" || mediaContentType == "song" || mediaContentType == "musicVideo";
}

void DiscogsInformationDetectorPlugin::contactServer(QString url, QString params, QString base) {
	if(!nam) {
		nam = new QNetworkAccessManager(this);
		connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));
	}

	QUrl finalUrl(
				params.isEmpty() ?
				base + url + "?key=" + key + "&secret=" + secret
				:
				base + url + "?" + params + "&key=" + key + "&secret=" + secret);
	nam->get(QNetworkRequest(finalUrl));
}

QNetworkReply * DiscogsInformationDetectorPlugin::get(const QString &path, const QString & params, const QString &base) {
	QNetworkAccessManager nam;
	return get(nam, path, params, base);
}

QNetworkReply * DiscogsInformationDetectorPlugin::get(QNetworkAccessManager & nam, const QString & path, const QString & params, const QString & base) {
	return PluginNetworkAccessHelper::get(nam, base + path + (params.isEmpty() ? "?key=" + key + "&secret=" + secret : "?" + params + "&key=" + key + "&secret=" + secret));
}

bool DiscogsInformationDetectorPlugin::lookForInformation(QNetworkAccessManager & nam, const QString & name, QMap<QString, QVariant> & info) {
	QJsonObject doc;
	QNetworkReply * reply = get(nam, "database/search", "type=artist&title="+QUrl::toPercentEncoding(name));
	doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8()).object();
	reply->deleteLater();
	QJsonArray results = doc["results"].toArray();
	const QString _name = QString(name).replace('-', ' ').replace('_', ' ').replace('.', ' ').toLower();
	const int len = results.count();
	for(int i = 0; i < len; i++) {
		//qDebug() << "getInformation(" << type << name << info << doc << ")";
		if(results.at(i).toObject()["title"].toString().replace('-', ' ').replace('_', ' ').replace('.', ' ').toLower() == _name) {
			int id = results.at(i).toObject()["id"].toInt(-1);
			if(id != -1) {
				reply = get(nam, "artists/" + QString::number(id));
				doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8()).object();
				reply->deleteLater();
				info["profile"] = doc["profile"].toString();
				QJsonArray images = doc["images"].toArray();
				if(images.count() > 0) {
					info["picture"] = images.at(0).toObject()["uri"].toString();
				}
				return true;
			}
		}
	}

	return false;
}

void DiscogsInformationDetectorPlugin::getInformation(const QString & type, const QMap<QString, QVariant> & known, QMap<QString, QVariant> & info) {
	if(type == "artist") {
		qDebug() << "Looking for info for" << known;
		QNetworkAccessManager nam;
		const QString name = known["name"].toString();
		if(!lookForInformation(nam, name, info) && known.contains("aka")) {
			for(auto aka : known["aka"].toStringList()) {
				if(lookForInformation(nam, aka, info)) return;
			}
		}
	}
}

void DiscogsInformationDetectorPlugin::onReplyFinished(QNetworkReply * reply) {
	QJsonDocument doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8());

	if(reply->url().path() == "/database/search") {
		QString result;
		QJsonArray results = doc.object()["results"].toArray();

		if(results.count() > 0) {
			QJsonObject obj = results[0].toObject();
//			if(obj["type"] == "artist") type = 0;
//			else if(obj["type"] == "master") type = 1;
//			else if(obj["type"] == "release") type = 2;
			contactServer(obj["resource_url"].toString(), "", "");
		} else {
			emit onInformationNotDetected(this);
		}
	} else {
		int len;
		QJsonObject obj = doc.object();

		currentContainer->title = obj["title"].toString();

		QJsonArray artists = obj["artists"].toArray();
		len = artists.count();
		for(int i = 0; i < len; i++) {
			currentContainer->artists.append(artists[i].toObject()["name"].toString());
		}

		if(obj.contains("year")) {
			int year = obj["year"].toInt(-1);
			if(year != -1) currentContainer->date = new ImpreciseDate(year);
		}

//		QJsonArray styles = obj["styles"].toArray();
//		if(styles.count() > 0) {
//			result += "<b>Styles:</b><ul>";
//			for(int i = 0; i < styles.count(); i++) {
//				result += "<li>" + styles[i].toString() + "</li>";
//			}
//			result += "</ul>";
//		}
//		QJsonArray urls = obj["urls"].toArray();
//		if(urls.count() > 0) {
//			result += "<b>Urls:</b><ul>";
//			for(int i = 0; i < urls.count(); i++) {
//				result += "<li>" + urls[i].toString() + "</li>";
//			}
//			result += "</ul>";
//		}
//		QJsonArray akas = obj["namevariations"].toArray();
//		if(akas.count() > 0) {
//			result += "<b>Also known as:</b><ul>";
//			for(int i = 0; i < akas.count(); i++) {
//				result += "<li>" + akas[i].toString() + "</li>";
//			}
//			result += "</ul>";
//		}
//		QJsonArray imgs = obj["images"].toArray();
//		if(imgs.count() > 0) {
//			result += "<b>Images:</b>";
//			for(int i = 0; i < imgs.count(); i++) {
//				//result += "<img src=\"" + imgs[i].toObject()["uri"].toString() + "\"/>";
//			}
//		}

		emit onInformationDetected(this);
	}

	reply->deleteLater();
}
