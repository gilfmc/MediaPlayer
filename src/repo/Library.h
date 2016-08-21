#ifndef LIBRARY_H
#define LIBRARY_H

#include <QtCore/QObject>
#include <QtSql/QSqlDatabase>

class Library;

#include "data/MediaProperty.h"
#include "repo/LibraryItemRegistrationListener.h"
#include "MediaContentLibrary.h"
#include "ArtistsLibrary.h"
#include "TvShowsLibrary.h"

#define SQL_FIELDS int _id; int id() const { return _id; }

unsigned int levenshteinDistance(const QString& s1, const QString& s2);

class Library : public QObject, private LibraryItemRegistrationListener {
	Q_OBJECT

	public:
		Library();
		
		MediaContentLibrary * mediaContents();
		ArtistsLibrary * artists();
		TvShowsLibrary * tvShows();

		virtual void onItemRegistered(const QString & type, MediaProperty * item) const;
		virtual void onItemAdded(const QString & type, MediaProperty * item) const;
		virtual void onItemRemoved(const QString & type, MediaProperty * item) const;
		virtual void onItemRenamed(const QString & type, MediaProperty * item, const QString & newName) const;
		virtual void onItemUnregistered(const QString & type, MediaProperty * item) const;

		MediaProperty * getMediaProperty(const QString & type, int id);

		Q_INVOKABLE
		void deleteLibrary(bool onlyMediaContents, void * from);
		
		virtual ~Library();
		
	signals:
		void onMediaPropertyRegistered(const QString & type, MediaProperty * item) const;
		void onMediaPropertyAddedToLibrary(const QString & type, MediaProperty * item) const;
		void onMediaPropertyRemovedFromLibrary(const QString & type, MediaProperty * item) const;
		void onMediaPropertyRenamed(const QString & type, MediaProperty * item, const QString & newName) const;
		void onMediaPropertyUnregistered(const QString & type, MediaProperty * item) const;
		
	private:
		QSqlDatabase db;
		
		MediaContentLibrary * _mediaContents = NULL;
		ArtistsLibrary * _artists = NULL;
		TvShowsLibrary * _tvShows = NULL;

		static const short version;

		void createDatabase(QSqlQuery & q);

	public:
		static QString toString(const ImpreciseDate & date);
		static ImpreciseDate toDate(const QString & date);
};

#endif // LIBRARY_H
