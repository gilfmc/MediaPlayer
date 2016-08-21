#ifndef MEDIAINFORMATIONSOURCEPLUGIN_H
#define MEDIAINFORMATIONSOURCEPLUGIN_H

#include <QtCore/QObject>

class MediaInformationSourcePlugin /* public QObject,*/ {
//		Q_OBJECT

	public:
		virtual void getInformation(const QString & type, const QMap<QString, QVariant> & known, QMap<QString, QVariant> & info) {}
};

Q_DECLARE_INTERFACE(MediaInformationSourcePlugin, "org.papyros.mediaplayer.MediaInformationSourcePlugin/1.0")

#endif // MEDIAINFORMATIONSOURCEPLUGIN_H
