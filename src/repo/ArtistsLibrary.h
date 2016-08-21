#ifndef ARTISTSLIBRARY_H
#define ARTISTSLIBRARY_H

#include <QtSql/QSqlRelationalTableModel>
#include <QtSql/QSqlQuery>

#include <QtCore/QDebug>
#include <QtCore/QMutex>

class ArtistsLibrary;
//class InternalArtistsLibrary;

#include "MediaPropertyLibrary.h"
#include "Library.h"
#include "data/Artist.h"
#include "data/Playlist.h"

//class ArtistsLibrary : public MediaPropertyLibrary<Artist*>
//{
//	public:
//		virtual const QList<Artist*> naturallySortedList() = 0;
		
//		virtual Artist * registerArtist(Artist * artist, Playlist * from) = 0;
//		virtual Artist * lookForArtist(QString name, int threshold = 4) = 0;
//		virtual QList<Artist *> lookForArtists(QString name, int threshold = 4) = 0;
		
//	protected:
//		ArtistsLibrary() {}
//};

class ArtistsLibrary : public MediaPropertyLibrary<Artist*> {
	public:
		ArtistsLibrary(Library * library, QSqlDatabase& db);
		
		virtual int count(bool libraryOnly=true);
		virtual Artist* get(qint64 id);
		virtual Artist* get(QString name);
		virtual Artist* lookForArtist(QString name, int threshold = 4);
		virtual QList<Artist *> lookForArtists(QString name, int threshold = 4);

		bool isInLibrary(Artist * item);

//		virtual Iterator * iterate();
		const QList<Artist*> naturallySortedList(bool inLibraryOnly = true);

		virtual Artist* registerArtist(Artist* artist, Playlist* from);

		virtual const QString & contentType() { return _type; }

		virtual void deleteAll(void * from);
		
		virtual bool add(Artist*);
		virtual bool update(Artist*);
		virtual bool remove(Artist*);
		virtual bool remove(int id);

	protected:
		bool setItemInLibrary(Artist *, bool);
		bool setItemName(Artist *, const QString &);

	private:
		const QSqlDatabase db;
		Library * library;
		
		int _count = -1;
		QMap<int, Artist*> loaded;
		QMap<int, Artist*> loadedLibrary;
		
		void load();
		
		QMutex mutex;

		static const QString _type;
};

#endif // ARTISTSLIBRARY_H
