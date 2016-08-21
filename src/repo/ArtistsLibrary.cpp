#include "ArtistsLibrary.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtConcurrent/QtConcurrent>

#include <stdarg.h>

#include "MediaPlayerContext.h"

const QString ArtistsLibrary::_type = "artist";

class SqlArtistAkas : public AlsoKnownAsElement {
	public:
		virtual const QString & get(int index) const { return names.at(index); }
		virtual int size() const { return names.length(); }
		
		void add(QString name) { names.append(name); }

	private:
		QStringList names;
};

class SqlArtist : public Artist {
	public:
		SQL_FIELDS
		
		SqlArtist(Library * library, Artist * a) : library(library), _name(a->name()) {
			_alsoKnownAs = new SqlArtistAkas();
			if(a->alsoKnownAs()) {
				AlsoKnownAsElement * sourceAkas = a->alsoKnownAs();
				const int len = sourceAkas->size();
				for(int i = 0; i < len; i++) {
					_alsoKnownAs->add(sourceAkas->get(i));
				}
			}
			addSynonyms(3, "&", "+", " and ");
			addSynonyms(2, "1", " one ");
			addSynonyms(2, "2", " two ");
			addSynonyms(2, "3", " three ");
			addSynonyms(2, "4", " four ");
			addSynonyms(2, "5", " five ");
			addSynonyms(2, "10", " ten ");
			addSynonyms(2, "20", " twenty ");
			addSynonyms(2, "30", " thirty ");
			addSynonyms(2, "40", " forty ");
			addSynonyms(2, "50", " fifty ");
			if(_name.startsWith("the ", Qt::CaseInsensitive))
				addAka(QString(_name).remove(0, 4));
		}
		SqlArtist(Library * library, int id, QString name) : library(library), _id(id), _name(name) {}

		void addSynonyms(int count, ...) {
			va_list vars;
			va_start(vars, count);
			QStringList synonyms;
			for(int i = 0; i < count; i++) {
				synonyms.append(QString(va_arg(vars, char*)));
			}
			va_end(vars);
			QString original = QChar(' ') + _name + QChar(' ');
			for(int i = 0; i < count; i++) {
				if(original.contains(synonyms[i], Qt::CaseInsensitive)) {
					for(int j = 0; j < count; j++) {
						if(j != i) addAka(QString(original).replace(synonyms[i], synonyms[j], Qt::CaseInsensitive).trimmed());
					}
				}
			}
		}

//		void addSynonyms(QString & name, QString synonyms[], int size, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
//			for(int i = 0; i < size; i++) {
//				if(name.contains(synonyms[i], cs)) {
//					for(int j = 0; j < size; j++) {
//						addAka(name.replace(synonyms[i], synonyms[j], cs).simplified());
//					}
//				}
//			}
//		}
		
		virtual const QString & name() const { return _name; }
		virtual AlsoKnownAsElement * alsoKnownAs() {
			if(!_alsoKnownAs) {
				QSqlQuery q;
				q.prepare("SELECT name FROM artistAkas WHERE artist=?");
				q.bindValue(0, _id);
				q.exec();
				_alsoKnownAs = new SqlArtistAkas();
				while(q.next()) {
					_alsoKnownAs->add(q.value(0).toString());
				}
			}
			
			return _alsoKnownAs;
		}

		virtual const QList<MediaContent*> getMediaContents() {
			QSqlQuery q;
			q.prepare("SELECT media FROM mediaSongs WHERE artist=?");
			q.bindValue(0, _id);
			q.exec();

			QList<MediaContent*> medias;
			MediaContentLibrary * mcl = library->mediaContents();
			while(q.next()) {
				medias.append(mcl->get(q.value(0).toInt()));
			}
			return medias;
		}

		virtual const QList<MediaContent*> featuredIn() {
			QSqlQuery q;
			q.prepare("SELECT media FROM artistsFeaturingMedia WHERE artist=?");
			q.bindValue(0, _id);
			q.exec();

			QList<MediaContent*> medias;
			MediaContentLibrary * mcl = library->mediaContents();
			while(q.next()) {
				medias.append(mcl->get(q.value(0).toInt()));
			}
			return medias;
		}

		void addAka(QString aka) {
			aka = aka.simplified();
			if(!_alsoKnownAs->contains(aka)) {
				_alsoKnownAs->add(aka);
			}
		}
		
		virtual ~SqlArtist() {
			if(_alsoKnownAs) delete _alsoKnownAs;
		}

		QString _name;
	private:
		Library * library;
		SqlArtistAkas * _alsoKnownAs = NULL;
};

typedef struct MeasuredArtistName {
	MeasuredArtistName() {}
	MeasuredArtistName(int measure, Artist * artist) : measure(measure), artist(artist) {}
	MeasuredArtistName(const MeasuredArtistName & man) : measure(man.measure), artist(man.artist) {}
	
	int measure;
	Artist * artist;
} MeasuredArtistName;

typedef struct StringDistanceMeasurer {
	StringDistanceMeasurer(QString string) : string(string.toLower()) {}

	typedef MeasuredArtistName result_type;
	MeasuredArtistName operator()(Artist * const & other) {
		QString o = other->name().toLower();
		// TODO find closer distances in the AKAs
		return MeasuredArtistName(levenshteinDistance(string, o), other);
	}

	const QString string;
} StringDistanceMeasurer;

typedef struct StringMatchDistanceMeasurer {
	StringMatchDistanceMeasurer(QString string) : string(string.simplified().toLower()) {}

	typedef MeasuredArtistName result_type;
	MeasuredArtistName operator()(Artist * const & other) {
		QString o = other->name().simplified().toLower().left(string.length());
		// TODO find closer distances in the AKAs
		return MeasuredArtistName(levenshteinDistance(string, o), other);
	}

	const QString string;
} StringMatchDistanceMeasurer;

ArtistsLibrary::ArtistsLibrary(Library * library, QSqlDatabase & db) : library(library), MediaPropertyLibrary((LibraryItemRegistrationListener*)library), db(db) { }

void ArtistsLibrary::load() {
	QMutexLocker locker(&mutex);
	if(_count != -1) return;
	
	QSqlQuery q("SELECT * FROM artists");
	loaded.clear();
	loadedLibrary.clear();
	int id;
	while(q.next()) {
		id = q.value(0).toInt();
		Artist * a = new SqlArtist(library, id, q.value(2).toString());
		loaded[id] = a;
		if(q.value(1).toBool()) loadedLibrary[id] = a;
	}
	_count = loaded.count();
}

int ArtistsLibrary::count(bool libraryOnly) {
	if(_count == -1) load();
	
	return libraryOnly ? loadedLibrary.count() : _count;
}

//MediaPropertyLibrary< Artist* >::Iterator* ArtistsLibrary::iterate() {
//// 	return new ArtistsLibraryIterator(QSqlQuery("SELECT * FROM artists"));
//}

const QList<Artist*> ArtistsLibrary::naturallySortedList(bool inLibraryOnly) {
	if(_count == -1) load();
	
	QList<Artist*> artists = inLibraryOnly ? loadedLibrary.values() : loaded.values();
	std::sort(artists.begin(), artists.end(), [](const Artist * a, const Artist * b) -> bool { return QString::localeAwareCompare(a->name(), b->name()) < 0; });
	return artists;
}

Artist* ArtistsLibrary::lookForArtist(QString name, int threshold) {
	if(_count == -1) load();

	if(_count > 0) {
		//qDebug() << "lookForArtist(" << name << threshold << ")";
		QList<Artist*> artists = loaded.values();
		QList<MeasuredArtistName> distances = QtConcurrent::blockingMapped(artists, StringDistanceMeasurer(name));
		std::sort(distances.begin(), distances.end(), [](const MeasuredArtistName & a, const MeasuredArtistName & b) -> bool { return a.measure < b.measure; });
		if(distances[0].measure < threshold && (_count == 1 || distances[0].measure == 0 || distances[0].measure != distances[1].measure)) {
			return distances[0].artist;
		}
	}

	return NULL;
}

QList<Artist*> ArtistsLibrary::lookForArtists(QString name, int threshold) {
	if(_count == -1) load();

	if(_count > 0) {
		QList<Artist*> artists = loaded.values();
		QList<MeasuredArtistName> distances = QtConcurrent::blockingMapped (
												  artists,
												  StringMatchDistanceMeasurer(name));
//		std::remove_if(distances.begin(), distances.end(), [threshold](const MeasuredArtistName & m) -> bool { return m.measure > threshold; });
		int len = distances.length();
		for(int i = 0; i < len; i++) {
			if(distances.at(i).measure > threshold) {
				distances.removeAt(i);
				i--; len--;
			}
		}
		std::sort(distances.begin(), distances.end(), [](const MeasuredArtistName & a, const MeasuredArtistName & b) -> bool { return a.measure < b.measure; });
		artists.clear();
		for(const MeasuredArtistName & man : distances) artists.append(man.artist);
		return artists;
	}

	return QList<Artist*>();
}

Artist* ArtistsLibrary::get(qint64 id) {
	if(_count == -1) load();
	
	return loaded.contains(id) ? loaded[id] : NULL;
}

Artist* ArtistsLibrary::get(QString name) {
	if(_count == -1) load();
	
	foreach (Artist * a, loaded) {
		if (a->name() == name) return a;
	}

	return NULL;
}

void ArtistsLibrary::deleteAll(void * from) {
	for(Artist * a : loaded) listener->onItemUnregistered("artist", a);
	QSqlQuery q(db);
	q.exec("DELETE FROM artistAkas");
	q.exec("DELETE FROM artists");
	loadedLibrary.clear();
	for(Artist * a : loaded) delete a;
	loaded.clear();
}

Artist* ArtistsLibrary::registerArtist(Artist* artist, Playlist * from) {
	QMutexLocker locker(&mutex);
	
	if (from == NULL || from->context()->library()->artists() == this) {
		SqlArtist * a = new SqlArtist(library, artist);
		delete artist;
		qDebug() << "Artist" << a->name() << "about to be registered by the playlist";
		Artist * existing;
		if((existing = get(a->name()))) {
			qDebug() << "Ignoring," << a->name() << "already exists...";
			return existing;
		} else {
			if (add(a)) {
				listener->onItemRegistered("artist", a);
				return a;
			}
		}
	}

	delete artist;

	return NULL;
}

bool ArtistsLibrary::setItemInLibrary(Artist * a, bool inLibrary) {
	QSqlQuery q(db);
	q.prepare("UPDATE artists SET inLibrary=? WHERE id=?");
	q.bindValue(0, inLibrary);
	q.bindValue(1, a->id());
	if(q.exec()) {
		if(inLibrary) loadedLibrary[a->id()] = a;
		else loadedLibrary.remove(a->id());
		return true;
	}
	return false;
}

bool ArtistsLibrary::setItemName(Artist * a, const QString & name) {
	for(Artist * artist : loaded) {
		if(artist == a) {
			QSqlQuery q(db);
			q.prepare("UPDATE artists SET name=? WHERE id=?");
			q.bindValue(0, name);
			q.bindValue(1, a->id());
			((SqlArtist*)a)->_name = name;
			if(q.exec()) {
				return true;
			} else {
				return false;
			}
		}
	}

	return false;
}

bool ArtistsLibrary::isInLibrary(Artist * item) {
	QSqlQuery q(db);
	q.prepare("SELECT inLibrary FROM artists WHERE id=?");
	q.bindValue(0, item->id());
	if(q.exec() && q.next()) return q.value(0).toBool();
	return false;
}

bool ArtistsLibrary::add(Artist * a) {
	QSqlQuery q(db);
	q.prepare("INSERT INTO artists (inLibrary, name) VALUES (?, ?)");
	q.bindValue(0, true);
	q.bindValue(1, a->name());
	if(q.exec()) {
		const int id = q.lastInsertId().toInt();
		((SqlArtist*)a)->_id = id;
		loaded[id] = a;
		loadedLibrary[id] = a;
		_count++;
		AlsoKnownAsElement * sourceAkas = a->alsoKnownAs();
		const int len = sourceAkas->size();
		for(int i = 0; i < len; i++) {
			q.prepare("INSERT INTO artistAkas (artist, name) VALUES (?, ?)");
			q.bindValue(0, id);
			q.bindValue(1, sourceAkas->get(i));
			q.exec();
		}
		return true;
	}

	qDebug() << "Failed to add artist:" << q.lastError();

	return false;
}

bool ArtistsLibrary::update(Artist*) {

}

bool ArtistsLibrary::remove(Artist*) {

}

bool ArtistsLibrary::remove(int id) {

}
