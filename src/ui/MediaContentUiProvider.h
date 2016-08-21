#ifndef MEDIACONTENTUIPROVIDER_H
#define MEDIACONTENTUIPROVIDER_H

#include <QtQml/QtQml>

class MediaContentUiProvider;

#include "MediaPlayerContext.h"
#include "data/MediaContent.h"
#include "MediaListUiProvider.h"

class MediaContentUiProviderInterface {
	public:
		virtual void notifyDoneLoadingInformation() = 0;
};

class MediaContentUiModelItem : public QObject {
	Q_OBJECT
	Q_PROPERTY(int number READ getNumber)
	Q_PROPERTY(const QVariant & title READ getTitle)

	public:
		MediaContentUiModelItem(MediaPlayerContext * context, MediaProperty * media) : context(context), media(media), id(media->id()), title(media->name()) { }

		virtual int getId() const { return id; }
		virtual int getNumber() const { return -1; }
		virtual const QString getTitle() { return title; }
		virtual const QString getDescription() { return ""; }
		virtual const QString getLength() { return ""; }
		virtual const QString getDate() { return ""; }
		Q_INVOKABLE
		virtual const QVariant getColumn(int column) {}

		MediaProperty * media;

		int id;
		QString title;

		virtual ~MediaContentUiModelItem() { }

	protected:
		MediaPlayerContext * context;
};

class MediaContentUiModelGroupItem : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(char type READ getType)
	Q_PROPERTY(bool showNumber READ showNumber)
	Q_PROPERTY(const QString & title READ getTitle)
	Q_PROPERTY(int columnCount READ getColumnCount)
//	Q_PROPERTY(const QString & emptyMessage READ getEmptyMessage)
//	Q_PROPERTY(const QString & emptyIcon READ getEmptyIcon)

	public:
		MediaContentUiModelGroupItem() { }
		MediaContentUiModelGroupItem(char type, QString title) :
			type(type), title(title), columnCount(0) { }
		MediaContentUiModelGroupItem(char type, QString title, int columnCount) :
			type(type), title(title), columnCount(columnCount) { }

		char type;
		bool _showNumber = true;
		QString title;
		int columnCount;

		QString emptyMessage, emptyIcon;

		char getType() { return type; }
		const QString & getTitle() { return title; }
		int getColumnCount() { return columnCount; }
		int showNumber() { return _showNumber; }
		void setShowNumber(bool show) { _showNumber = show; }

//		const QString & getEmptyMessage() { return emptyMessage; }
//		const QString & getEmptyIcon() { return emptyIcon; }

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;

		QList<MediaContentUiModelItem*> items;

		virtual ~MediaContentUiModelGroupItem() {
			for(MediaContentUiModelItem * item : items) delete item;
		}
};

class MediaContentUiModel : public QAbstractListModel {
	Q_OBJECT

	public:
		MediaContentUiModel(MediaPlayerContext * context, MediaProperty * property, QObject * parent);

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;

		Q_INVOKABLE
		MediaContentUiModelGroupItem * getGroupItem(int index);

		Q_INVOKABLE
		MediaListUiProvider * getListUiProvider(int index);

		Q_INVOKABLE
		void playAll(int index);
		Q_INVOKABLE
		void play(int groupIndex, int index);
		Q_INVOKABLE
		void addToPlaylist(int groupIndex, int index);

		virtual ~MediaContentUiModel() {
			for(MediaContentUiModelGroupItem * item : items) delete item;
		}

	private slots:
		void onMediaPropertyRegistered(const QString, MediaProperty *);

	private:
		MediaPlayerContext * context;
		MediaProperty * property;

		QList<MediaContentUiModelGroupItem*> items;

		MediaListUiProvider * makeListUiProvider(int index);
};

class MediaContentUiProvider : public QObject, private MediaContentUiProviderInterface {
	Q_OBJECT

	Q_PROPERTY(const QString & name READ name NOTIFY nameChanged)
	Q_PROPERTY(const QString & description READ description NOTIFY descriptionChanged)
	Q_PROPERTY(const QString & posterUrl READ posterUrl NOTIFY posterUrlChanged)

	Q_PROPERTY(int mediaId READ mediaId)
	Q_PROPERTY(const QString & mediaType READ mediaType)

	Q_PROPERTY(MediaContentUiModel * model READ model)

	public:
		explicit MediaContentUiProvider(MediaPlayerContext * context = 0, MediaProperty * media = 0, QObject * parent = 0);

		const QString & name();
		const QString description();
		const QString posterUrl();

		MediaContentUiModel * model();

		virtual void notifyDoneLoadingInformation();

		int mediaId();
		const QString & mediaType();

		virtual ~MediaContentUiProvider();

	signals:
		void nameChanged();
		void descriptionChanged();
		void posterUrlChanged();

	public slots:

	private:
		MediaPlayerContext * context;
		MediaProperty * media;
		bool loading = false;
		bool loadedInfo = false;
		QMap<QString, QVariant> info;

		MediaContentUiModel * _model;

		bool refreshingToRemakeCache = false;

		void load(bool refresh = false);
		
		void * loader = NULL;
};

#endif // MEDIACONTENTUIPROVIDER_H
