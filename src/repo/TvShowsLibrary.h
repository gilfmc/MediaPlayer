#ifndef TVSHOWSLIBRARY_H
#define TVSHOWSLIBRARY_H

#include <QtSql/QSqlRelationalTableModel>
#include <QtSql/QSqlQuery>

#include <QtCore/QMutex>

class TvShowsLibrary;

#include "MediaPropertyLibrary.h"
#include "Library.h"
#include "data/TvShow.h"

class WritableTvShow : public TvShow {
	public:
		virtual Season * addSeason(void * from, int number, QString name = "") = 0;
};

class TvShowsLibrary : public MediaPropertyLibrary<TvShow*>
{
	public:
		TvShowsLibrary(Library * library, QSqlDatabase& db);

		virtual int count(bool libraryOnly=true);
		virtual TvShow* get(qint64 id);
		virtual TvShow* get(QString name);
		virtual TvShow* lookForTvShow(QString name, int threshold = 4);
		virtual QList<TvShow*> lookForTvShows(QString name, int threshold = 4);

		bool isInLibrary(TvShow * item);

		const QList<TvShow*> naturallySortedList(bool inLibraryOnly = true);

		TvShow * registerTvShow(TvShow * tvShow, void * from);

		virtual const QString & contentType() { return _type; }

		virtual void deleteAll(void * from);

	protected:
		bool setItemInLibrary(TvShow *, bool);
		bool setItemName(TvShow *, const QString &);

	private:
		const QSqlDatabase db;
		Library * library;

		int _count = -1;
		QMap<int, TvShow*> loaded;
		QMap<int, TvShow*> loadedLibrary;

		void load();

		bool add(TvShow*);

		QMutex mutex;

		static const QString _type;
};

#endif // TVSHOWSLIBRARY_H
