#include "TvShowsLibrary.h"

#include <QtSql/QSqlError>
#include <QtConcurrent/QtConcurrent>

#include <algorithm>

const QString TvShowsLibrary::_type = "tvShow";

// TODO SqlTvShowAkas = SqlArtistAkas
class SqlTvShowAkas : public AlsoKnownAsElement {
	public:
		virtual const QString & get(int index) const { return names.at(index); }
		virtual int size() const { return names.length(); }

		void add(QString name) { names.append(name); }

	private:
		QStringList names;
};

class SqlSeason : public Season {
	public:
		SQL_FIELDS

		//SqlSeason(Season * s) : _name(s->name()), _number(s->number()) { }
		SqlSeason(Library * library, TvShow * tvShow, int number, QString name) : library(library), tvShow(tvShow), _number(number), _name(name) { }
		SqlSeason(Library * library, TvShow * tvShow, int id, int number, QString name) : library(library), tvShow(tvShow), _id(id), _number(number), _name(name) { }
		
		virtual int number() { return _number; }
		virtual const QString& name() const { return _name; }

		virtual MediaProperty * parent() { return tvShow; }
		
		virtual int episodeCount() {
			QSqlQuery q;
			q.prepare("SELECT COUNT(1) FROM (SELECT DISTINCT number FROM mediaTvEpisodes WHERE season=?)");
			q.bindValue(0, _id);
			if(q.exec() && q.next()) return q.value(0).toInt();
			return -1;
		}

		virtual const QList< Episode* > episodes() {
			QSqlQuery q;
			q.prepare("SELECT media FROM mediaTvEpisodes WHERE season=? ORDER BY number");
			q.bindValue(0, _id);
			q.exec();

			QList<Episode*> medias;
			MediaContentLibrary * mcl = library->mediaContents();
			while(q.next()) {
				medias.append((Episode*) mcl->get(q.value(0).toInt()));
			}
			return medias;
			//return _episodes;
		}
	private:
		Library * library;

		TvShow * tvShow;

		int _number;
		QString _name;
		bool episodesLoaded = false;
		//QList<Episode *> _episodes;
};

class SqlTvShow : public WritableTvShow {
	public:
		SQL_FIELDS
		
		SqlTvShow(Library * library, TvShow * a) : library(library), _name(a->name()) {
			if(a->alsoKnownAs()) {
				AlsoKnownAsElement * sourceAkas = a->alsoKnownAs();
				_alsoKnownAs = new SqlTvShowAkas();
				const int len = sourceAkas->size();
				for(int i = 0; i < len; i++) {
					_alsoKnownAs->add(sourceAkas->get(i));
				}
			}
		}
		SqlTvShow(Library * library, int id, QString name) : library(library), _id(id), _name(name) {}

		virtual const QString & name() const { return _name; }
		virtual AlsoKnownAsElement * alsoKnownAs() {
			if(!_alsoKnownAs) {
				QSqlQuery q;
				q.prepare("SELECT name FROM tvShowAkas WHERE tvShow=?");
				q.bindValue(0, _id);
				q.exec();
				_alsoKnownAs = new SqlTvShowAkas();
				while(q.next()) {
					_alsoKnownAs->add(q.value(0).toString());
				}
			}

			return _alsoKnownAs;
		}

		virtual Season * getSeason(int number) {
			foreach(Season * s, seasons()) {
				if(s->number() == number) return s;
			}

			return NULL;
		}

		virtual const QList<Season*> & seasons() {
			QMutexLocker locker(&mutex);

			if(loadSeasons) {
				foreach(Season * s, _seasons) delete s;
				_seasons.clear();

				QSqlQuery q;
				q.prepare("SELECT * FROM tvShowSeasons WHERE tvShow=?");
				q.bindValue(0, _id);
				q.exec();
				int id;
				while(q.next()) {
					id = q.value(0).toInt();
					_seasons.append(new SqlSeason(library, this, id, q.value(2).toInt(), q.value(3).toString()));
				}

				sortSeasons();

				loadSeasons = false;
			}

			return _seasons;
		}

		virtual Season * addSeason(void * from, int number, QString name = "") {
			QMutexLocker locker(&mutex);

			SqlSeason * season = new SqlSeason(library, this, number, name);
			qDebug() << "Season" << season->number() << "of" << _name << "about to be registered by" << from;
			if(exists(season)) {
				qDebug() << "This season already exists";
				return season;
			}

			QSqlQuery q;
			q.prepare("INSERT INTO tvShowSeasons (tvShow, number, name) VALUES (?, ?, ?)");
			q.bindValue(0, _id);
			q.bindValue(1, season->number());
			q.bindValue(2, season->name());
			if(q.exec()) {
				const int id = q.lastInsertId().toInt();
				season->_id = id;
				if(!loadSeasons) {
					_seasons.append(season);
					sortSeasons();
				}
				return season;
			}

			qDebug() << "Failed to add season:" << q.lastError();

			delete season;
			return NULL;
		}

		virtual const QList<MediaContent*> getMediaContents() {
			QSqlQuery q;
			q.prepare("SELECT e.media FROM mediaTvEpisodes e, tvShowSeasons s WHERE e.season=s.id AND s.tvShow=? ORDER BY s.number, e.number");
			q.bindValue(0, _id);
			q.exec();

			QList<MediaContent*> medias;
			MediaContentLibrary * mcl = library->mediaContents();
			while(q.next()) {
				medias.append(mcl->get(q.value(0).toInt()));
			}
			return medias;
		}
		
		virtual ~SqlTvShow() {
			if(_alsoKnownAs) delete _alsoKnownAs;
		}

		QString _name;
	private:
		Library * library;

		SqlTvShowAkas * _alsoKnownAs = NULL;
		bool loadSeasons = true;
		QList<Season *> _seasons;

		void sortSeasons() {
			std::sort(_seasons.begin(), _seasons.end(), [](Season* const& a, Season* const& b) { return a->number() < b->number(); });
		}

		bool exists(Season * season) {
			QSqlQuery q;
			q.prepare("SELECT COUNT(1) FROM tvShowSeasons WHERE tvShow=? AND number=?");
			q.bindValue(0, _id);
			q.bindValue(1, season->number());
			if(q.exec() && q.next() && q.value(0).toInt() > 0) return true;
			return false;
		}

		QMutex mutex;
};

// TODO: just like the AKAs, we can refactor this and reuse a lot of code
typedef struct MeasuredTvShowName {
	MeasuredTvShowName() {}
	MeasuredTvShowName(int measure, TvShow * tvShow) : measure(measure), tvShow(tvShow) {}
	MeasuredTvShowName(const MeasuredTvShowName & man) : measure(man.measure), tvShow(man.tvShow) {}

	int measure;
	TvShow * tvShow;
} MeasuredTvShowName;

typedef struct StringDistanceMeasurer {
	StringDistanceMeasurer(QString string) : string(string.toLower()) {}

	typedef MeasuredTvShowName result_type;
	MeasuredTvShowName operator()(TvShow * const & other) {
		QString o = other->name().toLower();
		// TODO find closer distances in the AKAs
		return MeasuredTvShowName(levenshteinDistance(string, o), other);
		}

	const QString string;
} StringDistanceMeasurer;

typedef struct StringMatchDistanceMeasurer {
	StringMatchDistanceMeasurer(QString string) : string(string.simplified().toLower()) {}

	typedef MeasuredTvShowName result_type;
	MeasuredTvShowName operator()(TvShow * const & other) {
		QString o = other->name().simplified().toLower().left(string.length());
		// TODO find closer distances in the AKAs
		return MeasuredTvShowName(levenshteinDistance(string, o), other);
	}

	const QString string;
} StringMatchDistanceMeasurer;

TvShowsLibrary::TvShowsLibrary(Library * library, QSqlDatabase & db) : library(library), MediaPropertyLibrary((LibraryItemRegistrationListener*)library), db(db) { }

void TvShowsLibrary::load() {
	QMutexLocker locker(&mutex);
	if(_count != -1) return;

	QSqlQuery q("SELECT * FROM tvShows");
	loaded.clear();
	loadedLibrary.clear();
	int id;
	while(q.next()) {
		id = q.value(0).toInt();
		TvShow * ts = new SqlTvShow(library, id, q.value(2).toString());
		loaded[id] = ts;
		if(q.value(1).toBool()) loadedLibrary[id] = ts;
	}
	_count = loaded.count();
}

int TvShowsLibrary::count(bool libraryOnly) {
	if(_count == -1) load();

	return libraryOnly ? loadedLibrary.count() : _count;
}

const QList<TvShow*> TvShowsLibrary::naturallySortedList(bool inLibraryOnly) {
	if(_count == -1) load();

	QList<TvShow*> TvShows = inLibraryOnly ? loadedLibrary.values() : loaded.values();
	std::sort(TvShows.begin(), TvShows.end(), [](const TvShow * a, const TvShow * b) -> bool { return QString::localeAwareCompare(a->name(), b->name()) < 0; });
	return TvShows;
}

TvShow* TvShowsLibrary::lookForTvShow(QString name, int threshold) {
	//qDebug() << "lookForTvShow(" << name << "," << threshold << ")";
	if(_count == -1) { /*qDebug() << "The DB needs to be loaded";*/ load(); }

	if(_count > 0) {
		QList<TvShow*> tvShows = loaded.values();
		name = name.toLower();
		//qDebug() << "We need to find" << name << "in" << tvShows.count() << "items";
		QList<MeasuredTvShowName> distances = QtConcurrent::blockingMapped(tvShows, StringDistanceMeasurer(name));
		std::sort(distances.begin(), distances.end(), [](const MeasuredTvShowName & a, const MeasuredTvShowName & b) -> bool { return a.measure < b.measure; });
		if(distances[0].measure < threshold && (_count == 1 || distances[0].measure == 0 || distances[0].measure != distances[1].measure)) {
			// TODO should sort by year, the newest first
			return distances[0].tvShow;
		}
	}

	// maybe the query has an year
	QRegularExpression year("(19\\d\\d)|(20[01]\\d)");
	QRegularExpressionMatch match = year.match(name);
	if(match.hasMatch()) {
		name = name.remove(match.captured(0)).trimmed();
		if(name.length() > 0) {
			// TODO should check if it is actually the queried year
			return lookForTvShow(name, threshold);
		}
	}

	return NULL;
}

QList<TvShow*> TvShowsLibrary::lookForTvShows(QString name, int threshold) {
	if(_count == -1) load();

	if(_count > 0) {
		QList<TvShow*> tvShows = loaded.values();
		name = name.toLower();
		//qDebug() << "We need to find" << name << "in" << tvShows.count() << "items";
		QList<MeasuredTvShowName> distances = QtConcurrent::blockingMapped(tvShows, StringMatchDistanceMeasurer(name));
		int len = distances.length();
		for(int i = 0; i < len; i++) {
			if(distances.at(i).measure > threshold) {
				distances.removeAt(i);
				i--; len--;
			}
		}
		std::sort(distances.begin(), distances.end(), [](const MeasuredTvShowName & a, const MeasuredTvShowName & b) -> bool { return a.measure < b.measure; });
		tvShows.clear();
		for(const MeasuredTvShowName & man : distances) tvShows.append(man.tvShow);
		return tvShows;
	}

	return QList<TvShow*>();
}

TvShow* TvShowsLibrary::get(qint64 id) {
	if(_count == -1) load();

	return loaded.contains(id) ? loaded[id] : NULL;
}

TvShow* TvShowsLibrary::get(QString name) {
	if(_count == -1) load();

	foreach (TvShow * a, loaded) {
		if (a->name() == name) return a;
	}

	return NULL;
}

void TvShowsLibrary::deleteAll(void * from) {
	for(TvShow * t : loaded) listener->onItemUnregistered("tvShow", t);
	QSqlQuery q(db);
	q.exec("DELETE FROM tvShowSeasons");
	q.exec("DELETE FROM tvShowAkas");
	q.exec("DELETE FROM tvShows");
	loadedLibrary.clear();
	for(TvShow * t : loaded) delete t;
	loaded.clear();
}

TvShow * TvShowsLibrary::registerTvShow(TvShow * tvShow, void * from) {
	QMutexLocker locker(&mutex);

	SqlTvShow * ts = new SqlTvShow(library, tvShow);
	delete tvShow;
	qDebug() << "TV show" << ts->name() << "about to be registered by" << from;
	TvShow * existing;
	if((existing = get(ts->name()))) {
		qDebug() << "Ignoring," << ts->name() << "already exists...";
		return existing;
	} else {
		if(add(ts)) {
			listener->onItemRegistered("tvShow", ts);
			return ts;
		}
	}

	return NULL;
}

bool TvShowsLibrary::setItemInLibrary(TvShow * ts, bool inLibrary) {
	QSqlQuery q(db);
	q.prepare("UPDATE tvShows SET inLibrary=? WHERE id=?");
	q.bindValue(0, inLibrary);
	q.bindValue(1, ts->id());
	if(q.exec()) {
		if(inLibrary) loadedLibrary[ts->id()] = ts;
		else loadedLibrary.remove(ts->id());
		return true;
	}
	return false;
}

bool TvShowsLibrary::setItemName(TvShow * ts, const QString & name) {
	for(TvShow * tvShow : loaded) {
		if(tvShow == ts) {
			QSqlQuery q(db);
			q.prepare("UPDATE tvShows SET name=? WHERE id=?");
			q.bindValue(0, name);
			q.bindValue(1, ts->id());
			((SqlTvShow*)ts)->_name = name;
			if(q.exec()) {
				return true;
			} else {
				return false;
			}
		}
	}

	return false;
}

bool TvShowsLibrary::isInLibrary(TvShow * item) {
	QSqlQuery q(db);
	q.prepare("SELECT inLibrary FROM tvShows WHERE id=?");
	q.bindValue(0, item->id());
	if(q.exec() && q.next()) return q.value(0).toBool();
	return false;
}

bool TvShowsLibrary::add(TvShow * ts) {
	QSqlQuery q(db);
	q.prepare("INSERT INTO tvShows (inLibrary, name) VALUES (?, ?)");
	q.bindValue(0, true);
	q.bindValue(1, ts->name());
	if(q.exec()) {
		const int id = q.lastInsertId().toInt();
		((SqlTvShow*)ts)->_id = id;
		loaded[id] = ts;
		loadedLibrary[id] = ts;
		_count++;
		AlsoKnownAsElement * sourceAkas = ts->alsoKnownAs();
		const int len = sourceAkas->size();
		for(int i = 0; i < len; i++) {
			q.prepare("INSERT INTO tvShowAkas (artist, name) VALUES (?, ?)");
			q.bindValue(0, id);
			q.bindValue(1, sourceAkas->get(i));
			q.exec();
		}
		return true;
	}

	qDebug() << "Failed to add TV show:" << q.lastError();

	return false;
}
