#ifndef MEDIAPROPERTY_H
#define MEDIAPROPERTY_H

#include <QtCore/QString>

class MediaProperty {
	public:
		virtual int id() const = 0;
		virtual const QString & name() const = 0;
		virtual const QString & type() const = 0;
};

#endif // MEDIAPROPERTY_H
