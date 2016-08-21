#ifndef MEDIAPROPERTYLIBRARY_H
#define MEDIAPROPERTYLIBRARY_H

#include <QtCore/QObject>
//#include <QtCore/QListIterator>

#include "repo/LibraryItemRegistrationListener.h"

template<class T>
class MediaPropertyLibrary {
	public:
		MediaPropertyLibrary(LibraryItemRegistrationListener * listener = NULL) : listener(listener) { }

		virtual int count(bool libraryOnly=true) = 0;
		virtual T get(qint64 id) = 0;
		virtual T get(QString name) = 0;

		virtual bool isInLibrary(T item) = 0;
		virtual void addToLibrary(T item) {
			if(setItemInLibrary(item, true) && listener) {
				listener->onItemAdded(contentType(), item);
			}
		}
		virtual void removeFromLibrary(T item) {
			if(setItemInLibrary(item, false) && listener) {
				listener->onItemRemoved(contentType(), item);
			}
		}

		virtual void rename(T item, const QString & newName) {
			if(setItemName(item, newName) && listener) {
				listener->onItemRenamed(contentType(), item, newName);
			}
		}

		virtual const QString & contentType() = 0;

		virtual void deleteAll(void * from) = 0;
		
//		class Iterator {
//			public:
//				virtual T item() = 0;
//				virtual bool next() = 0;

//				virtual ~Iterator() {}
//		};
		
//		virtual Iterator * iterate() = 0;
	protected:
		virtual bool setItemInLibrary(T, bool) = 0;
		virtual bool setItemName(T, const QString &) = 0;

		const LibraryItemRegistrationListener * listener;
};

template<class T>
class WritableMediaPropertyLibrary { // : public MediaPropertyLibrary<T> {
	public:
		virtual bool add(T) = 0;
		virtual bool update(T) = 0;
		virtual bool remove(T) = 0;
		virtual bool remove(int id) = 0;
};

#endif // MEDIAPROPERTYLIBRARY_H
