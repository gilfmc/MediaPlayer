#ifndef MEDIACONTENTLIBRARY_H
#define MEDIACONTENTLIBRARY_H

#include <QtSql/QSqlRelationalTableModel>
#include <QtSql/QSqlQuery>

#include <QtCore/QMutex>

class MediaContentLibrary;

#include "data/MediaContent.h"
#include "MediaPropertyLibrary.h"
#include "Library.h"

typedef struct MediaContentSearchOptions {
		enum SortOptions { DontSort, LastAddedFirst };

		QString query;
		QString type;
		//int limit = 0;
		SortOptions sort = DontSort;
} MediaContentSearchOptions;

class MediaContentLibrary : public MediaPropertyLibrary<MediaContent*>
{
	public:
		MediaContentLibrary(Library * library, QSqlDatabase& db);

		virtual int count(bool libraryOnly=true);
		virtual MediaContent* get(qint64 id);
		virtual MediaContent* get(QString uri);
		//virtual MediaContent* lookForTvShow(QString name, int threshold = 4);
		virtual QList<MediaContent*> search(MediaContentSearchOptions & options);

		bool isInLibrary(MediaContent * item);

		virtual bool has(QString uri);

		int registerMedia(MediaContent * media, void * from);

		virtual const QString & contentType() { return _type; }
		
		void updateMedia(MediaContent * media, qint64 duration, void * from);

		void deleteAll(void * from);

	protected:
		bool setItemInLibrary(MediaContent *, bool);
		bool setItemName(MediaContent *, const QString &);

	private:
		QSqlDatabase db;
		Library * library;

		QMutex mutex;

		MediaContent * load(QSqlQuery &);

		QString removeAccents(const QString & string);

		QString accentedCharacters;
		QStringList baseCharacters;

		static const QString _type;
};

#endif // MEDIACONTENTLIBRARY_H
