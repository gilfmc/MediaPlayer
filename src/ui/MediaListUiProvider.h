#ifndef MEDIALISTUIPROVIDER_H
#define MEDIALISTUIPROVIDER_H

#include <QtQml/QtQml>

#include "data/Song.h"
#include "data/TvShow.h"

#define MLUP_DESTRUCTOR(className, container) virtual ~className() { qDebug() << "About to clean up" << #className; const int len = container.length(); for(int i = 0; i < len; i++) delete container[i]; container.clear(); }

class MediaListUiProvider : public QAbstractListModel {
	Q_OBJECT
	
	typedef struct Column {
		Column(QString name) : name(name), width(-1) { }
		QString name;
		int width;
	} Column;

	public:
		explicit MediaListUiProvider() { }

// 		Q_INVOKABLE
// 		QString getColumnName(int index);

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const { return 0; }
		Q_INVOKABLE
		virtual QVariant value(int row, int column) { return QVariant(); }
		virtual QHash< int, QByteArray > roleNames() const;
		
		int columnCount(const QModelIndex & parent) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;

		Q_INVOKABLE
		virtual int getMediaContentId(int index) { return -1; }

		QString formatLength(MediaContent * media) const;

		virtual ~MediaListUiProvider();
	
	protected:
		QList<Column> columns;
		
		Column & addColumn(QString name, int index = -1);
};

class SongsListUiProvider : public MediaListUiProvider {
	public:
		SongsListUiProvider(Artist * artist);

		int rowCount(const QModelIndex &parent) const;
		QVariant value(int row, int column);

		int getMediaContentId(int index);

		MLUP_DESTRUCTOR(SongsListUiProvider, songs)

	private:
		QList<Song*> songs;
};

class EpisodesListUiProvider : public MediaListUiProvider {
	public:
		EpisodesListUiProvider(TvShow * tvShow);

		int rowCount(const QModelIndex & parent) const;
		QVariant value(int row, int column);

		int getMediaContentId(int index);

		MLUP_DESTRUCTOR(EpisodesListUiProvider, episodes)

	private:
		QList<Episode*> episodes;
};

class SeasonsListUiProvider : public MediaListUiProvider {
	public:
		SeasonsListUiProvider(TvShow * tvShow);

		int rowCount(const QModelIndex & parent) const;
		QVariant value(int row, int column);

	private:
		QList<Season*> seasons;
};

#endif // MEDIALISTUIPROVIDER_H
