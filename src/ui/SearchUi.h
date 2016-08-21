#ifndef SEARCHUI_H
#define SEARCHUI_H

#include <QtQml/QtQml>

#include "MediaPlayerContext.h"

class SearchGroup : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(int itemLimit READ itemLimit WRITE setItemLimit)

	public:
		QString title;
		int type;

		SearchGroup() { }
		SearchGroup(const SearchGroup & from) : SearchGroup(from.title, from.type) { }
		SearchGroup(const QString & title, int type) : title(title), type(type) { }

		virtual QVariant data(int index, int field) const { }
		virtual int count() const { return 0; }

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
			return data(index.row(), role - Qt::UserRole);
		}
		int rowCount(const QModelIndex& parent = QModelIndex()) const {
			const int realCount = count();
			if(limit >= 0 && realCount > limit) return limit;
			else return realCount;
		}
		virtual QHash< int, QByteArray > roleNames() const { }

		virtual void search(const QString & query) { }

		virtual MediaProperty * getItem(int) { return NULL; }

		int itemLimit() { return limit; }
		void setItemLimit(int itemLimit) {
			const int before = rowCount();
			limit = itemLimit;
			const int after = rowCount();
			if(before > after) {
				beginRemoveRows(QModelIndex(), after, before-1);
				endRemoveRows();
			} else if(after > before) {
				beginInsertRows(QModelIndex(), before, after-1);
				endInsertRows();
			}
		}

	protected:
		int limit = -1;
};

class ThumbnailsSearchGroup : public SearchGroup {
	public:
		ThumbnailsSearchGroup(const QString & title) : SearchGroup(title, 4) { }

		virtual QHash< int, QByteArray > roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "id";
			roles[Qt::UserRole + 1] = "number";
			roles[Qt::UserRole + 2] = "title";
			roles[Qt::UserRole + 3] = "description";
			roles[Qt::UserRole + 4] = "length";
			roles[Qt::UserRole + 5] = "date";
			roles[Qt::UserRole + 6] = "thumbnail";
			return roles;
		}
};

class PeopleSearchGroup : public SearchGroup {
	public:
		PeopleSearchGroup(const QString & title) : SearchGroup(title, 3) { }

		virtual QHash< int, QByteArray > roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "id";
			roles[Qt::UserRole + 1] = "title";
			roles[Qt::UserRole + 2] = "picture";
			return roles;
		}
};

class CoversSearchGroup : public SearchGroup {
	public:
		CoversSearchGroup(const QString & title) : SearchGroup(title, 2) { }

		virtual QHash< int, QByteArray > roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "id";
			roles[Qt::UserRole + 1] = "number";
			roles[Qt::UserRole + 2] = "title";
			roles[Qt::UserRole + 3] = "description";
			roles[Qt::UserRole + 4] = "length";
			roles[Qt::UserRole + 5] = "date";
			roles[Qt::UserRole + 6] = "picture";
			return roles;
		}
};

class SimpleListSearchGroup : public SearchGroup {
	public:
		SimpleListSearchGroup(const QString & title) : SearchGroup(title, 1) { }

		virtual QHash< int, QByteArray > roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "id";
			roles[Qt::UserRole + 1] = "number";
			roles[Qt::UserRole + 2] = "title";
			roles[Qt::UserRole + 3] = "description";
			roles[Qt::UserRole + 4] = "length";
			roles[Qt::UserRole + 5] = "date";
			return roles;
		}
};

class SearchUi : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(MediaPlayerContext * context READ context WRITE setContext)

	public:
		SearchUi();
		SearchUi(const SearchUi &);

		Q_INVOKABLE
		void search(const QString & query);

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash<int, QByteArray> roleNames() const;

		Q_INVOKABLE
		SearchGroup * getGroup(int index);

		MediaPlayerContext * context();
		void setContext(MediaPlayerContext * context);

		Q_INVOKABLE
		MediaContentUiProvider * getItemUi(int group, int index);

		~SearchUi();

	private:
		MediaPlayerContext * _context = NULL;
		QList<SearchGroup*> groups;
};

#endif // SEARCHUI_H
