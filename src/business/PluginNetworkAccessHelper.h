#ifndef PLUGINNETWORKACCESSHELPER_H
#define PLUGINNETWORKACCESSHELPER_H

#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class PluginNetworkAccessHelper {
	public:
		static QNetworkReply * get(QNetworkAccessManager & nam, const QString & path) {
			QUrl url(path);
			QEventLoop eventLoop;
			QObject::connect(&nam, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
			qDebug() << "Loading" << path << "...";
			QNetworkReply * reply = nam.get(QNetworkRequest(url));
			// qDebug() << "Done";
			eventLoop.exec();
			return reply;
		}
};

#endif // PLUGINNETWORKACCESSHELPER_H
