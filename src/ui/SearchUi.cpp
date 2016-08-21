#include "SearchUi.h"

class ArtistsGroup : public PeopleSearchGroup {
	public:
		ArtistsGroup(MediaPlayerContext * context) : PeopleSearchGroup(QObject::tr("Artists")), artists(context->library()->artists()) { }

		virtual QVariant data(int index, int field) const {
			switch(field) {
				case 0: return found.at(index)->id();
				case 1: return found.at(index)->name();
				case 2: return "image://mp/artist/" + QString::number(found.at(index)->id());
			}
			return QVariant();
		}

		virtual int count() const {
			return found.length();
		}

		virtual void search(const QString & query) {
			beginResetModel();
			//found.clear();
			found = artists->lookForArtists(query, query.length()/3);
			//while(found.length() > 10) found.removeLast();
			//if(a) found.append(a);
			endResetModel();
			//emit dataChanged(index(0), index(found.length()-1));
		}

		virtual MediaProperty * getItem(int index) {
			return found.at(index);
		}

	private:
		ArtistsLibrary * artists;
		QList<Artist*> found;
};

class SongsGroup : public SimpleListSearchGroup {
	public:
		SongsGroup(MediaPlayerContext * context) : SimpleListSearchGroup(QObject::tr("Songs")), context(context), library(context->library()->mediaContents()) {
			options.type = "song";
		}

		virtual QVariant data(int index, int field) const {
			switch(field) {
				case 0: return found.at(index)->id();
				//case 1: return ((Song*)found.at(index))->number();
				case 2: return found.at(index)->name();
				case 3: {
					Artist * a = ((Song*)found.at(index))->artist();
					if(a) return a->name();
					return "";
				}
				case 4: return context->formatLength(found.at(index));
			}
			return QVariant();
		}

		virtual int count() const {
			return found.length();
		}

		virtual void search(const QString & query) {
			options.query = query;
			QList<MediaContent*> newFound = library->search(options);
			while(newFound.length() > 10) {
				delete newFound.takeLast();
			}
			const int l = found.length();
			const int nl = newFound.length();
			if(nl > l) {
				beginInsertRows(QModelIndex(), l, nl-1);
				endInsertRows();
				for(MediaContent * m : found) delete m;
				found = newFound;
				emit dataChanged(index(l), index(nl-1));
			} else if (nl < l) {
				beginRemoveRows(QModelIndex(), nl, l-1);
				endRemoveRows();
				for(MediaContent * m : found) delete m;
				found = newFound;
				emit dataChanged(index(l), index(nl-1));
			} else {
				bool didChange = false;
				for(int i = 0; i < l; i++) {
					if(found.at(i)->id() != newFound.at(i)->id()) {
						didChange = true;
						break;
					}
				}
				for(MediaContent * m : found) delete m;
				found = newFound;
				if(didChange) emit dataChanged(index(0), index(nl-1));
			}
		}

		virtual ~SongsGroup() {
			for(MediaContent * m : found) delete m;
		}
	private:
		MediaPlayerContext * context;
		MediaContentLibrary * library;
		QList<MediaContent*> found;
		MediaContentSearchOptions options;
};

class TvShowsGroup : public CoversSearchGroup {
	public:
		TvShowsGroup(MediaPlayerContext * context) : CoversSearchGroup(QObject::tr("TV shows")), tvShows(context->library()->tvShows()) { }

		virtual QVariant data(int index, int field) const {
			switch(field) {
				case 0: return found.at(index)->id();
				case 2: return found.at(index)->name();
				//case 5: return found.at(index)->
				case 6: return "image://mp/tvShow/" + QString::number(found.at(index)->id());
			}
			return QVariant();
		}

		virtual int count() const {
			return found.length();
		}

		virtual void search(const QString & query) {
			beginResetModel();
			found = tvShows->lookForTvShows(query, query.length()/3);
			//while(found.length() > 10) found.removeLast();
			endResetModel();
		}

		virtual MediaProperty * getItem(int index) {
			return found.at(index);
		}
	private:
		TvShowsLibrary * tvShows;
		QList<TvShow*> found;
};

class TvShowEpisodesGroup : public ThumbnailsSearchGroup {
	public:
		TvShowEpisodesGroup(MediaPlayerContext * context) : ThumbnailsSearchGroup(QObject::tr("Episodes")), context(context), library(context->library()->mediaContents()) {
			options.type = "episode";
		}

		virtual QVariant data(int index, int field) const {
			switch(field) {
				case 0: return found.at(index)->id();
				case 1: return ((Episode*)found.at(index))->number();
				case 2: {
					Episode * e = (Episode*)found.at(index);
					if(e->name().isEmpty()) return QObject::tr("Episode %1").arg(e->number());
					else return QString::number(e->number()) + " - " + e->name();
				}
				case 3: {
					Season * s = ((Episode*)found.at(index))->season();
					if(s) {
						if(s->name().isEmpty()) return QObject::tr("Season %1").arg(s->number());
						else return s->name();
					}
					return "";
				}
				case 4: return context->formatLength(found.at(index));
				//case 5: return ((Episode*)found.at(index))->date().year();
				case 5: {
					Season * s = ((Episode*)found.at(index))->season();
					if(s) {
						const MediaProperty * ts = s->parent();
						if(ts) return ts->name();
					}
					return "";
				}
			}
			return QVariant();
		}

		virtual int count() const {
			return found.length();
		}

		virtual void search(const QString & query) {
			options.query = query;
			QList<MediaContent*> newFound = library->search(options);
			//while(newFound.length() > 20) newFound.removeLast();
			const int l = rowCount(); // found.length();
			for(MediaContent * m : found) delete m;
			found = newFound;
			const int nl = rowCount(); // newFound.length();
			if(nl > l) {
				beginInsertRows(QModelIndex(), l, nl-1);
				endInsertRows();
//				for(MediaContent * m : found) delete m;
//				found = newFound;
//				emit dataChanged(index(l), index(nl-1));
			} else if (nl < l) {
				beginRemoveRows(QModelIndex(), nl, l-1);
				endRemoveRows();
//				for(MediaContent * m : found) delete m;
//				found = newFound;
//				emit dataChanged(index(l), index(nl-1));
			} /*else {*/
//				bool didChange = false;
//				for(int i = 0; i < l; i++) {
//					if(found.at(i)->id() != newFound.at(i)->id()) {
//						didChange = true;
//						break;
//					}
//				}
				/*if(didChange) */emit dataChanged(index(0), index(nl-1));
//			}
		}

		virtual ~TvShowEpisodesGroup() {
			for(MediaContent * m : found) delete m;
		}

	private:
		MediaPlayerContext * context;
		MediaContentLibrary * library;
		QList<MediaContent*> found;
		MediaContentSearchOptions options;
};

SearchUi::SearchUi() {}

SearchUi::SearchUi(const SearchUi & from) {
	_context = from._context;
	groups = from.groups;
}

void SearchUi::search(const QString & query) {
	if(query.isEmpty()) return;

	qDebug() << "Searching" << query;
	for(SearchGroup * group : groups) group->search(query);
	QVector<int> roles;
	roles << Qt::UserRole+2;
	emit dataChanged(index(0), index(groups.length()-1), roles);
}

QVariant SearchUi::data(const QModelIndex & index, int role) const {
	int i = index.row();

	role -= Qt::UserRole;
	switch(role) {
		case 0: return groups.at(i)->type;
		case 1: return groups.at(i)->title;
		case 2: return groups.at(i)->rowCount();
	}

	return QVariant();
}

int SearchUi::rowCount(const QModelIndex &) const {
	return groups.length();
}

QHash< int, QByteArray > SearchUi::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "type";
	roles[Qt::UserRole + 1] = "title";
	roles[Qt::UserRole + 2] = "count";
	return roles;
}

SearchGroup * SearchUi::getGroup(int index) {
	SearchGroup * group = groups.at(index);
	QQmlEngine::setObjectOwnership(group, QQmlEngine::CppOwnership);
	return group;
}

void SearchUi::setContext(MediaPlayerContext * context) {
	if(_context == NULL) {
		_context = context;
		SearchGroup * sg;
		groups.append((sg = new ArtistsGroup(context)));
		groups.append(new SongsGroup(context));
		groups.append(new TvShowsGroup(context));
		groups.append(new TvShowEpisodesGroup(context));
	}
}

MediaPlayerContext * SearchUi::context() {
	return _context;
}

MediaContentUiProvider * SearchUi::getItemUi(int group, int index) {
	MediaProperty * m = groups.at(group)->getItem(index);
	return m ? new MediaContentUiProvider(_context, m) : NULL;
}

SearchUi::~SearchUi() {
	qDebug() << "Deleting this SearchUi...";
	for(SearchGroup * group : groups) delete group;
}
