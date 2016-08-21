#ifndef INFORMATIONDETECTORPLUGIN_H
#define INFORMATIONDETECTORPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "data/ImpreciseDate.h"

class MediaContentInformationContainer {
	public:
		QString mediaName;
		qint64 duration;

		short type;

		QString title;
		QStringList artists;
		QString album;
		ImpreciseDate * date;

		~MediaContentInformationContainer() {
			if(date) delete date;
		}
};

class ElementToCheck {
	public:
		ElementToCheck(QString type, QString sourceName) : type(type), sourceName(sourceName) {}

		///
		/// \brief artist, tvshow, movie, etc.
		///
		const QString type;
		///
		/// \brief the string we're checking (for example, the file name)
		///
		const QString sourceName;
		///
		/// \brief if a match was found, the "real" name of what we're looking for
		///
		QString	realName;
};

class InformationDetectorPlugin : public QObject {
		Q_OBJECT

	public:
		///
		/// \brief The player will request this plugin to detect as much information as possible
		/// with the given data in MediaContentInformationContainer. This will be run on a separate
		/// thread and the plugin should either emit onInformationDetected() or
		/// onInformationNotDetected()
		/// \param knownInfo a container with the already known information, should always have,
		/// at least, the name of the media (file name) and duration, it may contain info detected
		/// with other plugins.
		///
		virtual void detectInformation(MediaContentInformationContainer * knownInfo) { emit onInformationNotDetected(this); }

		///
		/// \brief Checks if a given string is found in a database containing the given type (Artist, TV Show, Movie, etc.)
		/// \param element a container with the data to check and the results of that check
		///
		virtual void checkExistence(ElementToCheck * element) { emit onInformationNotDetected(this); }

		///
		/// \brief Whether this InformationDetectorPlugin supports the specified mediaContentType
		/// \param mediaContentType, the type of media content such as "song", "tvShow", "movie"
		/// or any other type that can be added with plugins or new versions
		/// \return true if it supports this mediaContentType, false otherwise
		///
		virtual bool supports(QString mediaContentType) { return false; }

		virtual ~InformationDetectorPlugin() {}

	signals:
		void onInformationDetected(InformationDetectorPlugin * sender);
		void onInformationNotDetected(InformationDetectorPlugin * sender);
};

Q_DECLARE_INTERFACE(InformationDetectorPlugin, "org.papyros.mediaplayer.InformationDetectorPlugin/1.0")

#endif // INFORMATIONDETECTORPLUGIN_H
