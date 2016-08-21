#include "MediaListUiProvider.h"

SongsListUiProvider::SongsListUiProvider(Artist * artist) {
	for(MediaContent * media : artist->getMediaContents()) {
		this->songs.append((Song*) media);
	}

//	std::sort(this->songs.begin(), this->songs.end(), [](Artist* const& a, Artist* const& b) {
//		const int seasonA = a->season() ? a->season()->number() : INT32_MAX;
//		const int seasonB = b->season() ? b->season()->number() : INT32_MAX;

//		if(seasonA == seasonB) return a->number() < b->number();
//		else return seasonA < seasonB;
//	});

	addColumn("#").width = 40;
	addColumn(tr("Title")).width = -1;
	addColumn(tr("Album")).width = -3;
	addColumn(tr("Length")).width = -3;
}

int SongsListUiProvider::rowCount(const QModelIndex& parent) const {
	songs.length();
}

QVariant SongsListUiProvider::value(int row, int column) {
	if(row < 0 || row >= songs.length()) return QVariant();
	
	switch(column) {
		case 0: return songs[row]->number();
		case 1: return songs[row]->name();
		case 2: return QVariant("");
		case 3: return formatLength(songs[row]);
	}

	return QVariant();
}

int SongsListUiProvider::getMediaContentId(int index) {
	return songs[index]->id();
}

EpisodesListUiProvider::EpisodesListUiProvider(TvShow * tvShow) {
	for(MediaContent * media : tvShow->getMediaContents()) {
		this->episodes.append((Episode*) media);
	}

	std::sort(this->episodes.begin(), this->episodes.end(), [](Episode* const& a, Episode* const& b) {
		const int seasonA = a->season() ? a->season()->number() : INT_MAX;
		const int seasonB = b->season() ? b->season()->number() : INT_MAX;

		if(seasonA == seasonB) return a->number() < b->number();
		else return seasonA < seasonB;
	});

	addColumn("#").width = 40;
	addColumn(tr("Title")).width = -1;
	addColumn(tr("Season")).width = -3;
	addColumn(tr("Date")).width = -3;
	addColumn(tr("Length")).width = -3;
}

int EpisodesListUiProvider::rowCount(const QModelIndex& parent) const {
	episodes.length();
}

QVariant EpisodesListUiProvider::value(int row, int column) {
	if(row < 0 || row >= episodes.length()) return QVariant();
	
	switch(column) {
		case 0: return episodes[row]->number();
		case 1: return episodes[row]->name();
		case 2: return episodes[row]->season() ? QVariant(episodes[row]->season()->number()) : QVariant("");
		case 3: {
			ImpreciseDate date = episodes[row]->date();
			return date.hasYear() ? QString::number(date.dayOfMonth()) + "/" + QString::number(date.month()) + "/" + QString::number(date.year()) : "";
		}
		case 4: return formatLength(episodes[row]);
	}

	return QVariant();
}

int EpisodesListUiProvider::getMediaContentId(int index) {
	return episodes[index]->id();
}

SeasonsListUiProvider::SeasonsListUiProvider(TvShow * tvShow) {
	for(Season * media : tvShow->seasons()) {
		this->seasons.append(media);
	}

	addColumn(tr("Season"));
	addColumn(tr("Episodes"));
}

int SeasonsListUiProvider::rowCount(const QModelIndex& parent) const {
	seasons.length();
}

QVariant SeasonsListUiProvider::value(int row, int column) {
	if(row < 0 || row >= seasons.length()) return QVariant();
	
	switch(column) {
		case 0: return seasons[row]->number();
		case 1: return seasons[row]->episodeCount();
	}

	return QVariant();
}

int MediaListUiProvider::columnCount(const QModelIndex& parent) const {
	return columns.length();
}

QVariant MediaListUiProvider::headerData(int section, Qt::Orientation orientation, int role) const {
	switch(role) {
		case 0: return columns[section].name;
		case 1: return columns[section].width;
	}

	return QVariant();
}

QVariant MediaListUiProvider::data(const QModelIndex& index, int role) const {
	return index.row();
}

QHash<int, QByteArray> MediaListUiProvider::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "i";
	//roles[Qt::UserRole + 1] = "type";
	//roles[Qt::UserRole + 2] = "title";
	return roles;
}

QString MediaListUiProvider::formatLength(MediaContent * media) const {
	if(media) {
		const qint64 len = media->length();
		if(len <= 0) return QString();

		const qint64 seconds = len/1000;
		const qint64 minutes = seconds/60;
		const qint64 hours = minutes/60;

		QString result;
		if(hours > 0) result += QString::number(hours) + ':';
		const int min = minutes - 60*hours;
		if(min < 10 && hours > 0) result += "0" + QString::number(min) + ':';
		else result += QString::number(min) + ':';
		const int sec = seconds - 60*minutes;
		if(sec < 10) result += "0" + QString::number(sec);
		else result += QString::number(sec);

		return result;
	}

	return QString();
}

MediaListUiProvider::Column & MediaListUiProvider::addColumn(QString name, int index) {
	if(index == -1) {
		index = columns.length();
		columns.append(Column(name));
	} else {
		columns.insert(index, Column(name));
	}
	return columns[index];
}

MediaListUiProvider::~MediaListUiProvider() {
}
