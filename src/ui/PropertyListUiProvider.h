#ifndef PROPERTYLISTUIPROVIDER_H
#define PROPERTYLISTUIPROVIDER_H

#include <QtCore/QList>
#include <QtQml/QtQml>

class PropertyListUiProvider;

#include "MediaPlayerContext.h"

class PropertyListUiProvider : public QAbstractListModel {
		Q_OBJECT
		Q_PROPERTY(QString readableCount READ readableCount NOTIFY readableCountChanged)
		Q_PROPERTY(int viewType READ viewType NOTIFY viewTypeChanged)
		Q_PROPERTY(QString groupName READ groupName NOTIFY groupNameChanged)
		
	public:
		PropertyListUiProvider();
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;

		virtual char viewType() { return 0; }
		
		virtual int length() const { return 0; }
		virtual QString readableCount() const { return ""; }
		
		Q_INVOKABLE
		virtual MediaContentUiProvider * getItemUi(int) const { return NULL; }
		virtual int id(int) const { return -1; }
		virtual QString title(int) const { return ""; }
		//Q_INVOKABLE
		virtual QString imageUrl(int) const { return ""; }
		virtual bool inLibrary(int) const { return true; }

		virtual QString groupName() const { return ""; }
		
		static const char typePeople, typeCards;
		
	signals:
		void readableCountChanged();
		void viewTypeChanged(char viewType);
		void groupNameChanged();
};

class GenericLibraryListUiProvider : public PropertyListUiProvider {
		Q_OBJECT
		Q_PROPERTY(bool showAll READ showAll WRITE setShowAll NOTIFY showAllChanged)

	public:
		GenericLibraryListUiProvider(MediaPlayerContext & context, MediaPropertyLibrary<MediaProperty*> * library);
		
		int length() const;
		
		MediaContentUiProvider * getItemUi(int) const;
		int id(int index) const;
		QString title(int index) const;
		QString imageUrl(int index) const;
		bool inLibrary(int index) const;

		bool showAll() const;
		void setShowAll(bool);

	signals:
		void showAllChanged(bool);

	private slots:
		void _onMediaPropertyAddedToLibrary(const QString &, MediaProperty *);
		void onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *);
		
	protected:
		MediaPlayerContext & context;
		MediaPropertyLibrary<MediaProperty*> * library;

		MediaInformationManager * mediaInformationManager() const;
		//MediaPropertyLibrary<TvShow*>
		QList<MediaProperty*> items;

		bool showItemsNotInTheLibrary = false;

		virtual void onMediaPropertyAddedToLibrary(const QString &, MediaProperty *) = 0;

		virtual void onShowAllChanged(bool) { }
};

class MusicListUiProvider : public GenericLibraryListUiProvider {
	public:
		MusicListUiProvider(MediaPlayerContext & context);
		
		char viewType();

		virtual QString readableCount() const;

		virtual QString groupName() const;

	protected:
		void onMediaPropertyAddedToLibrary(const QString &, MediaProperty *);
		void onShowAllChanged(bool);

	private:
		//ArtistsLibrary * library;
};

class TvShowsListUiProvider : public GenericLibraryListUiProvider {
	public:
		TvShowsListUiProvider(MediaPlayerContext & context);
		
		char viewType();

		virtual QString readableCount() const;

		virtual QString groupName() const;

	protected:
		void onMediaPropertyAddedToLibrary(const QString &, MediaProperty *);
		void onShowAllChanged(bool);
		
	private:
		//TvShowsLibrary * library;
};

#endif // PROPERTYLISTUIPROVIDER_H
