#ifndef DISCOGSINFORMATIONDETECTORPLUGIN_H
#define DISCOGSINFORMATIONDETECTORPLUGIN_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "discogsinformationdetectorplugin_global.h"

#include "../../../src/business/PluginNetworkAccessHelper.h"
#include "../../../src/business/InformationDetectorPlugin.h"
#include "../../../src/business/MediaInformationSourcePlugin.h"

class DISCOGSINFORMATIONDETECTORPLUGINSHARED_EXPORT DiscogsInformationDetectorPlugin
		: public InformationDetectorPlugin, public MediaInformationSourcePlugin
{
		Q_OBJECT
		Q_PLUGIN_METADATA(IID "org.papyros.mediaplayer.plugins.discogs/1.0" FILE "manifest.json")
		Q_INTERFACES(InformationDetectorPlugin)
		Q_INTERFACES(MediaInformationSourcePlugin)
	public:
		DiscogsInformationDetectorPlugin();

		void detectInformation(MediaContentInformationContainer * container);

		bool supports(QString mediaContentType);

		void getInformation(const QString & type, const QMap<QString, QVariant> & known, QMap<QString, QVariant> & info);

	private slots:
		void onReplyFinished(QNetworkReply * reply);

	private:
		QNetworkAccessManager * nam = NULL;
		//TODO it needs to support multithreading!
		MediaContentInformationContainer * currentContainer = NULL;

		QNetworkReply * get(const QString & path, const QString & params = "", const QString & base = "https://api.discogs.com/");
		QNetworkReply * get(QNetworkAccessManager & nam, const QString & path, const QString & params = "", const QString & base = "https://api.discogs.com/");

		bool lookForInformation(QNetworkAccessManager & nam, const QString & name, QMap<QString, QVariant> & info);

		void contactServer(QString url, QString params = QString(),
											 QString base = "https://api.discogs.com/");

		///
		/// \brief Discogs key, do not use it in other projects as this is
		/// project-specific (and is supposed to be private)
		///
		const QString key = "OyWDaFBcNkCCiWaKaZub";
		///
		/// \brief Discogs secret, do not use it in other projects as this is
		/// project-specific (and is supposed to be private)
		///
		const QString secret = "tJYNjjrkGCEXShVlrWpNNVaViEXYrTiK";
};

#endif // DISCOGSINFORMATIONDETECTORPLUGIN_H
