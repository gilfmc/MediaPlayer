#include "Library.h"

#include <QtSql/QSqlQuery>
#include <QtCore/QDebug>

// adapted from: https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
unsigned int levenshteinDistance(const QString& s1, const QString& s2) {
	const int len1 = s1.size(), len2 = s2.size();
	std::vector<unsigned int> col(len2+1), prevCol(len2+1);

	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i+1;
		for (unsigned int j = 0; j < len2; j++)
			col[j+1] = std::min({ prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i]==s2[j] ? 0 : 1) });
		col.swap(prevCol);
	}
	return prevCol[len2];
}

const short Library::version = 3;

Library::Library() {
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
	QString dataPath;
	if(paths.size() > 0) {
		QDir configDir(paths[0]);
		configDir.mkpath(".");
		dataPath = configDir.filePath("library.sqlite");
	} else {
		dataPath = "library.sqlite";
	}
	db.setDatabaseName(dataPath);
	if (!db.open()) {
		qDebug() << "ERROR: Couldn't load the database!";
	}

	int version;
	QSqlQuery q("SELECT value FROM metadata WHERE key='version'");
	if (q.next()) {
		version = q.value(0).toInt();
	} else {
		// the database isn't set up yet, let's set it up
		createDatabase(q);
	}
	
	if(version < Library::version) {
		// the database needs to be updated
		qDebug() << "Updating the database from version" << version << "to" << Library::version << "...";
		if(version <= 2) {
			q.exec("ALTER TABLE mediaContents ADD COLUMN dateAdded INTEGER");
			if(version <= 1) {
				q.exec("CREATE TABLE artistsFeaturingMedia (id INTEGER PRIMARY KEY AUTOINCREMENT, artist INTEGER NOT NULL, media INTEGER NOT NULL)");
			}
		}
		q.exec(QString("UPDATE metadata SET value=%1 WHERE key='version'").arg(Library::version));
	}
	
	this->db = db;
}

void Library::createDatabase(QSqlQuery &q) {
	qDebug() << "Setting up database...";
	q.exec("CREATE TABLE metadata (key VARCHAR(32) PRIMARY KEY, value TEXT)");

	q.exec("CREATE TABLE mediaContents (id INTEGER PRIMARY KEY AUTOINCREMENT, inLibrary INTEGER, uri TEXT, type TEXT, length INTEGER, name TEXT, dateAdded INTEGER)");
	q.exec("CREATE TABLE mediaSongs (id INTEGER PRIMARY KEY AUTOINCREMENT, media INTEGER, number INTEGER, album INTEGER, artist INTEGER)");
	q.exec("CREATE TABLE mediaTvEpisodes (id INTEGER PRIMARY KEY AUTOINCREMENT, media INTEGER, season INTEGER, number INTEGER, date TEXT)");
	q.exec("CREATE TABLE mediaExtraInfo (id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT, propertyId INTEGER, name TEXT, value TEXT, lastUpdate INTEGER)");

	q.exec("CREATE TABLE artists (id INTEGER PRIMARY KEY AUTOINCREMENT, inLibrary INTEGER, name TEXT)");
	q.exec("CREATE TABLE artistsFeaturingMedia (id INTEGER PRIMARY KEY AUTOINCREMENT, artist INTEGER NOT NULL, media INTEGER NOT NULL)");
	q.exec("CREATE TABLE artistAkas (id INTEGER PRIMARY KEY AUTOINCREMENT, artist INTEGER, name TEXT)");
	q.exec("CREATE TABLE musicAlbums (id INTEGER PRIMARY KEY AUTOINCREMENT, artist INTEGER, name TEXT, date TEXT)");

	q.exec("CREATE TABLE tvShows (id INTEGER PRIMARY KEY AUTOINCREMENT, inLibrary INTEGER, name TEXT)");
	q.exec("CREATE TABLE tvShowAkas (id INTEGER PRIMARY KEY AUTOINCREMENT, tvShow INTEGER, name TEXT)"); // is it worth it separating AKA tables?
	q.exec("CREATE TABLE tvShowSeasons (id INTEGER PRIMARY KEY AUTOINCREMENT, tvShow INTEGER, number INTEGER, name TEXT)");

	q.exec(QString("INSERT INTO metadata (key, value) VALUES ('version', %1)").arg(Library::version));
}

MediaContentLibrary * Library::mediaContents() {
	if(_mediaContents) return _mediaContents;

	_mediaContents = new MediaContentLibrary(this, db);
	return _mediaContents;
}

ArtistsLibrary * Library::artists() {
	if(_artists) return _artists;
	
	_artists = new ArtistsLibrary(this, db);
	return _artists;
}

TvShowsLibrary * Library::tvShows() {
	if(_tvShows) return _tvShows;

	_tvShows = new TvShowsLibrary(this, db);
	return _tvShows;
}

MediaProperty * Library::getMediaProperty(const QString & type, int id) {
	if(type == "artist") return artists()->get(id);
	else if(type == "tvShow") return tvShows()->get(id);
	else return NULL;
}

void Library::deleteLibrary(bool onlyMediaContents, void * from) {
	qDebug() << "deleteLibrary(" << onlyMediaContents << from << ")";
	if(!onlyMediaContents) {
		artists()->deleteAll(from);
		tvShows()->deleteAll(from);
	}
	mediaContents()->deleteAll(from);
}

void Library::onItemRegistered(const QString & type, MediaProperty * item) const {
	emit onMediaPropertyRegistered(type, item);
	emit onMediaPropertyAddedToLibrary(type, item);
}

void Library::onItemUnregistered(const QString &type, MediaProperty * item) const {
	emit onMediaPropertyUnregistered(type, item);
}

void Library::onItemAdded(const QString & type, MediaProperty * item) const {
	emit onMediaPropertyAddedToLibrary(type, item);
}

void Library::onItemRemoved(const QString & type, MediaProperty * item) const {
	emit onMediaPropertyRemovedFromLibrary(type, item);
}

void Library::onItemRenamed(const QString & type, MediaProperty * item, const QString & newName) const {
	emit onMediaPropertyRenamed(type, item, newName);
}

QString Library::toString(const ImpreciseDate & date) {
	QString result;
	if(date.hasYear()) result += QString::number(date.year());
	result += ";";
	if(date.hasMonth()) result += QString::number(date.month());
	result += ";";
	if(date.hasDayOfMonth()) result += QString::number(date.dayOfMonth());

	return result;
}

ImpreciseDate Library::toDate(const QString & date) {
	QStringList parts = date.split(';');
	if(parts.length() < 3) return ImpreciseDate();

	bool ok;
	int year = parts[0].toInt(&ok); if(!ok) year = -1;
	int month = parts[1].toInt(&ok); if(!ok) month = -1;
	int day = parts[2].toInt(&ok); if(!ok) day = -1;

	return ImpreciseDate(year, month, day);
}

Library::~Library() {
	db.close();
	
	delete _artists;
}
