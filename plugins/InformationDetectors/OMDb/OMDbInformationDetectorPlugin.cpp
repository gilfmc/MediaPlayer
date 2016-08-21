#include "OMDbInformationDetectorPlugin.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QEventLoop>

OMDbInformationDetectorPlugin::OMDbInformationDetectorPlugin()
{
}

void OMDbInformationDetectorPlugin::checkExistence(ElementToCheck * element) {
	const QString type = element->type;
	const bool isTvShow = type == "tvShow";
	const bool isMovie = isTvShow ? false : type == "movie";

	if(type == "video" || isTvShow || isMovie) {
		const QString typeToSearch = isTvShow ? "&type=series" : "&type=movie";

		QNetworkAccessManager nam;

		qDebug() << "name ==" << element->sourceName;
	//	QUrl url("http://www.omdbapi.com/?t="+QUrl::toPercentEncoding(element->sourceName)+"&y=&plot=full");
	//	QNetworkReply * reply = nam.get(QNetworkRequest(url));

	//	QEventLoop eventLoop;
	//	// also dispose the event loop after the reply has arrived
	//	connect(&nam, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
	//	eventLoop.exec();
		QNetworkReply * reply = get(nam, "t="+QUrl::toPercentEncoding(element->sourceName)+typeToSearch+"&plot=full");
		switch(onReplyFinished(reply, element)) {
			case 1:
				emit onInformationDetected(this);
				return;
			case 0: {
				// maybe we can do something to find it
				QRegularExpressionMatch match = regex.year.match(element->sourceName);
				bool ok = false;
				int year = match.hasMatch() ? match.captured().toInt(&ok) : 0;
				if(ok) {
					QString sourceName = element->sourceName;
					reply = get(nam, "t="+QUrl::toPercentEncoding(sourceName.remove(match.captured()).trimmed())+"&y="+QString::number(year)+"&plot=full");
					if(onReplyFinished(reply, element) == 1) {
						emit onInformationDetected(this);
					} else {
						emit onInformationNotDetected(this);
					}
				} else {
					emit onInformationNotDetected(this);
				}
				return;
			}
			case -1:
				// network error? API change?
				emit onInformationNotDetected(this);
				return;
		}
		//disconnect(&nam, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
	}

	emit onInformationNotDetected(this);
}

//QNetworkReply * OMDbInformationDetectorPlugin::get(const QString & path) {
//	return PluginNetworkAccessHelper::get("http://www.omdbapi.com/?" + path);
//}

QNetworkReply * OMDbInformationDetectorPlugin::get(QNetworkAccessManager & nam, const QString & path) {
	return PluginNetworkAccessHelper::get(nam, "http://www.omdbapi.com/?" + path);
//	QUrl url("http://www.omdbapi.com/?" + path);
//	QNetworkReply * reply = nam->get(QNetworkRequest(url));
//	QEventLoop eventLoop;
//	QObject::connect(nam, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
//	eventLoop.exec();
//	return reply;
}

char OMDbInformationDetectorPlugin::onReplyFinished(QNetworkReply * reply, ElementToCheck * element) {
	QJsonDocument doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8());
	char result;
	if(doc.isObject()) {
		QJsonObject obj = doc.object();
		if(obj.contains("Error")) {
			result = 0;

			if(cachedReply) delete cachedReply;
			cachedReply = NULL;
		} else {
			element->realName = obj["Title"].toString();
			result = 1;

			if(cachedReply) delete cachedReply;
			cachedReply = new QJsonObject(obj);
			cachedType = element->type;
			cachedName = element->realName;
		}
	} else {
		result = -1;

		if(cachedReply) delete cachedReply;
		cachedReply = NULL;
	}

	reply->deleteLater();

	return result;
}

void OMDbInformationDetectorPlugin::getInformation(const QString & type, const QMap<QString, QVariant> & known, QMap<QString, QVariant> & info) {
	const bool isEpisode = type == "episode";
	const bool isTvShow = isEpisode ? false : type == "tvShow";
	const bool isMovie = isEpisode || isTvShow ? false : type == "movie";

	if(isEpisode || isTvShow || isMovie) {
		const QString name = isEpisode ? known["tvShow"].toString() : known["name"].toString();
		const QString typeToSearch = isEpisode ? "&type=episode" : (isTvShow ? "&type=series" : "&type=movie");
		QJsonObject doc;
		QNetworkAccessManager nam;
//		if(cachedReply && cachedName == name && cachedType == type) {
//			doc = * cachedReply;
//		} else {
			QNetworkReply * reply = isEpisode ? get(nam, "t="+QUrl::toPercentEncoding(name)+"&season="+QString::number(known["season"].toInt())+"&episode="+QString::number(known["number"].toInt())+"&plot=full") : get(nam, "t="+QUrl::toPercentEncoding(name)+typeToSearch+"&plot=full");
			doc = QJsonDocument::fromJson(QString(reply->readAll()).toUtf8()).object();
			reply->deleteLater();
//		}

		if(!doc.isEmpty()) {
			info["plot"] = doc["Plot"].toString();
			const QString title = doc["Title"].toString();
			if(!isEpisode || title != QString("Episode #") + known["season"].toString() + "." + known["number"].toString()) info["name"] = title;
			if(isEpisode) {
				info["picture"] = doc["Poster"].toString().remove("._V1_SX300");
				const QString date = doc["Released"].toString();
				const QStringList parts = date.split(' ', QString::SkipEmptyParts);
				if(parts.length() == 3) {
					int day = parts[0].toInt();
					QString _month = parts[1].toLower();
					int month;
					int year = parts[2].toInt();
					QChar _m = _month.at(0);
					if(_m == 'j') {
						if(_month.at(1) == 'a') month = 1;
						else {
							if(_month.at(2) == 'n') month = 6;
							else month = 7;
						}
					}
					if(_m == 'f') month = 2;
					if(_m == 'm') {
						if(_month.at(2) == 'r') month = 3;
						else month = 5;
					}
					if(_m == 'a') {
						if(_month.at(1) == 'p') month = 4;
						else month = 8;
					}
					if(_m == 's') month = 9;
					if(_m == 'o') month = 10;
					if(_m == 'n') month = 11;
					if(_m == 'd') month = 12;
					info["date"] = QDate(year, month, day);
				}
			} else {
				info["poster"] = doc["Poster"].toString().remove("._V1_SX300");
			}
		}
	}
}
