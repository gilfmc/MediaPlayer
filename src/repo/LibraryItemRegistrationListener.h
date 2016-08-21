#ifndef LIBRARYITEMREGISTRATIONLISTENER_H
#define LIBRARYITEMREGISTRATIONLISTENER_H

#include "data/MediaProperty.h"

class LibraryItemRegistrationListener {
	public:
		virtual void onItemRegistered(const QString & type, MediaProperty * item) const = 0;
		virtual void onItemUnregistered(const QString & type, MediaProperty * item) const = 0;

		virtual void onItemAdded(const QString & type, MediaProperty * item) const = 0;
		virtual void onItemRemoved(const QString & type, MediaProperty * item) const = 0;

		virtual void onItemRenamed(const QString & type, MediaProperty * item, const QString & newName) const = 0;
};

#endif // LIBRARYITEMREGISTRATIONLISTENER_H
