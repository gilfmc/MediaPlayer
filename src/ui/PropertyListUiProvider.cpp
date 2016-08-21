#include "PropertyListUiProvider.h"

const char PropertyListUiProvider::typePeople = 1;
const char PropertyListUiProvider::typeCards = 2;

// Base

PropertyListUiProvider::PropertyListUiProvider() {

}

QVariant PropertyListUiProvider::data(const QModelIndex& index, int role) const {
	int i = index.row();
	
	role -= Qt::UserRole;
	switch(role) {
		case 0: return i;
		case 1: return id(i);
		case 2: return title(i);
		case 3: return imageUrl(i);
		case 4: return inLibrary(i);
	}
	
	return QVariant();
}

QHash<int, QByteArray> PropertyListUiProvider::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "i";
	roles[Qt::UserRole + 1] = "id";
	roles[Qt::UserRole + 2] = "title";
	roles[Qt::UserRole + 3] = "imageUrl";
	roles[Qt::UserRole + 4] = "inLibrary";
	return roles;
}

int PropertyListUiProvider::rowCount(const QModelIndex&) const {
	return length();
}

// Generic

#define GENERICLIBRARYLISTUIPROVIDER_INITIATOR(library) for(MediaProperty * p : library) items.append(p);

GenericLibraryListUiProvider::GenericLibraryListUiProvider(MediaPlayerContext & context, MediaPropertyLibrary<MediaProperty *> * library) : context(context), library(library) {
	connect(context.library(), SIGNAL(onMediaPropertyAddedToLibrary(const QString &, MediaProperty *)), this, SLOT(_onMediaPropertyAddedToLibrary(const QString &, MediaProperty *)));
	connect(context.library(), SIGNAL(onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *)), this, SLOT(onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *)));
	connect(context.library(), &Library::onMediaPropertyRenamed, [=](const QString & type, MediaProperty * item, const QString &) {
		if(type == groupName()) {
			const int len = items.length();
			for(int i = 0; i < len; i++) {
				if(items.at(i) == item) {
					QModelIndex mi = index(i);
					QVector<int> roles;
					roles << Qt::UserRole + 2;
					emit dataChanged(mi, mi, roles);
					return;
				}
			}
		}
	});
}

void GenericLibraryListUiProvider::_onMediaPropertyAddedToLibrary(const QString & type, MediaProperty * item) {
	if(type == groupName()) {
		for(MediaProperty * m : items) if(m == item) return;
		onMediaPropertyAddedToLibrary(type, item);
	}
}

void GenericLibraryListUiProvider::onMediaPropertyRemovedFromLibrary(const QString & type, MediaProperty * item) {
	if(showItemsNotInTheLibrary) return;

	const int len = items.length();
	for(int i = 0; i < len; i++) {
		if(items.at(i) == item) {
			beginRemoveRows(QModelIndex(), i, i);
			items.removeAt(i);
			endRemoveRows();
			return;
		}
	}
}

MediaContentUiProvider * GenericLibraryListUiProvider::getItemUi(int index) const {
	MediaProperty * m = items.at(index);
	return m ? new MediaContentUiProvider(&context, m) : NULL;
}

int GenericLibraryListUiProvider::length() const {
	return items.length();
}

int GenericLibraryListUiProvider::id(int index) const {
	MediaProperty * m = items.at(index);
	return m ? m->id() : -1;
}

QString GenericLibraryListUiProvider::title(int index) const {
	MediaProperty * m = items.at(index);
	return m ? m->name() : QString();
}

QString GenericLibraryListUiProvider::imageUrl(int index) const {
	MediaProperty * m = items.at(index);
	return m ? "image://mp/" + m->type() + '/' + QString::number(m->id()) : QString();
}

bool GenericLibraryListUiProvider::inLibrary(int index) const {
	return library->isInLibrary(items.at(index));
}

bool GenericLibraryListUiProvider::showAll() const {
	return showItemsNotInTheLibrary;
}

void GenericLibraryListUiProvider::setShowAll(bool showAll) {
	if(showItemsNotInTheLibrary != showAll) {
		showItemsNotInTheLibrary = showAll;
		beginResetModel();
		onShowAllChanged(showAll);
		emit showAllChanged(showAll);
		endResetModel();
	}
}

MediaInformationManager * GenericLibraryListUiProvider::mediaInformationManager() const {
	return context.mediaInformationManager();
}

// Music

MusicListUiProvider::MusicListUiProvider(MediaPlayerContext& context) : GenericLibraryListUiProvider(context, (MediaPropertyLibrary<MediaProperty*>*) context.library()->artists()) {
	GENERICLIBRARYLISTUIPROVIDER_INITIATOR(context.library()->artists()->naturallySortedList(!showItemsNotInTheLibrary));
}

char MusicListUiProvider::viewType() {
	return PropertyListUiProvider::typePeople;
}

QString MusicListUiProvider::readableCount() const {
	return MediaListUiProvider::tr("%n artist(s)", "", length());
}

QString MusicListUiProvider::groupName() const {
	return "artist";
}

void MusicListUiProvider::onMediaPropertyAddedToLibrary(const QString & type, MediaProperty * item) {
	int i = 0;
	Artist * a;
	foreach(a, context.library()->artists()->naturallySortedList()) {
		i++;
		if(a == item) break;
	}
	beginInsertRows(QModelIndex(), i, i);
	items.insert(i, item);
	endInsertRows();
}

void MusicListUiProvider::onShowAllChanged(bool showAll) {
	items.clear();
	GENERICLIBRARYLISTUIPROVIDER_INITIATOR(context.library()->artists()->naturallySortedList(!showAll));
}

// TV Shows

TvShowsListUiProvider::TvShowsListUiProvider(MediaPlayerContext& context) : GenericLibraryListUiProvider(context, (MediaPropertyLibrary<MediaProperty*>*) context.library()->tvShows()) {
	GENERICLIBRARYLISTUIPROVIDER_INITIATOR(context.library()->tvShows()->naturallySortedList(!showItemsNotInTheLibrary));
}

char TvShowsListUiProvider::viewType() {
	return PropertyListUiProvider::typeCards;
}

QString TvShowsListUiProvider::readableCount() const {
	return MediaListUiProvider::tr("%n TV shows(s)", "", length());
}

QString TvShowsListUiProvider::groupName() const {
	return "tvShow";
}

void TvShowsListUiProvider::onMediaPropertyAddedToLibrary(const QString & type, MediaProperty * item) {
	int i = 0;
	TvShow * ts;
	foreach(ts, context.library()->tvShows()->naturallySortedList()) {
		i++;
		if(ts == item) break;
	}
	beginInsertRows(QModelIndex(), i, i);
	items.insert(i, item);
	endInsertRows();
}

void TvShowsListUiProvider::onShowAllChanged(bool showAll) {
	items.clear();
	GENERICLIBRARYLISTUIPROVIDER_INITIATOR(context.library()->tvShows()->naturallySortedList(!showAll));
}
