#ifndef OMDBINFORMATIONDETECTORPLUGIN_H
#define OMDBINFORMATIONDETECTORPLUGIN_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtCore/QRegularExpression>
#include <QtCore/QJsonObject>

#include "omdb_global.h"

#include "../../../src/business/PluginNetworkAccessHelper.h"
#include "../../../src/business/InformationDetectorPlugin.h"
#include "../../../src/business/MediaInformationSourcePlugin.h"

class OMDBSHARED_EXPORT OMDbInformationDetectorPlugin
		: public InformationDetectorPlugin, public MediaInformationSourcePlugin
{
		Q_OBJECT
		Q_PLUGIN_METADATA(IID "org.papyros.mediaplayer.plugins.omdb/1.0" FILE "manifest.json")
		Q_INTERFACES(InformationDetectorPlugin)
		Q_INTERFACES(MediaInformationSourcePlugin)
	public:
		OMDbInformationDetectorPlugin();

		void checkExistence(ElementToCheck * element);

		void getInformation(const QString & type, const QMap<QString, QVariant> & known, QMap<QString, QVariant> & info);

	//private slots:
		//void onReplyFinished(QNetworkReply * reply);

	private:
		// QRegularExpression precompiles regular expressions. In order to improve performance (and reduce unnecessary memory usage),
		// let's reuse them all the time.
		class RegexUtils {
			public:
				RegexUtils () :
//					groupDelimiters(QRegularExpression("(\\(.+\\))|(\\[.+\\])")),
					year(QRegularExpression("(19\\d\\d)|(20[01]\\d)"))
					{ }

				const QRegularExpression /*groupDelimiters,*/ year;
		};

		//QNetworkAccessManager * nam = NULL;

		//ElementToCheck * currentElement;

		//QNetworkReply * get(QNetworkAccessManager * nam, const QString & path);
		//QNetworkReply * get(const QString & path);
		QNetworkReply * get(QNetworkAccessManager & nam, const QString & path);
		char onReplyFinished(QNetworkReply * reply, ElementToCheck * currentElement);
		const RegexUtils regex;

		QJsonObject * cachedReply = NULL;
		QString cachedType, cachedName;
};

#endif // OMDBINFORMATIONDETECTORPLUGIN_H
