#include "MediaContentLibrary.h"

#include <QtSql/QSqlError>

const QString MediaContentLibrary::_type = "mediaContent";

class SqlMediaContent : public MediaContent {
	public:
		SqlMediaContent(int id, QUrl uri, QString name, qint64 length) :
						MediaContent(id, uri, name, length) {}

		virtual const QString & type() const { return QString::null; }
		
		void setLength(qint64 length) { _length = length; }
};

class SqlSong : public Song {
	public:
		SqlSong(const MediaContent & m, Library * library, int number, Artist * artist) :
						Song(m), _number(number), _artist(artist), library(library) {}

		virtual int number() { return _number; }
		virtual Artist * artist() { return _artist; }
		virtual QList<Artist *> & featuring() {
			if(_featuring) return * _featuring;
			else {
				QSqlQuery q;
				q.prepare("SELECT artist FROM artistsFeaturingMedia WHERE media=?");
				q.bindValue(0, _id);
				q.exec();
				_featuring = new QList<Artist*>();
				while(q.next()) {
					Artist * a = library->artists()->get(q.value(0).toInt());
					if(a) _featuring->append(a);
				}
				return * _featuring;
			}
		}

		virtual ~SqlSong() {
			if(_featuring) delete _featuring;
		}

	private:
		int _number;
		Artist * _artist;
		QList<Artist *> * _featuring = NULL;

		Library * library;
};

class SqlEpisode : public Episode {
	public:
		SqlEpisode(const MediaContent & m, TvShow * tvShow, int season, int number, ImpreciseDate date) :
			Episode(m), _tvShow(tvShow), _season(season), _number(number), _date(date) { }

		virtual int number() { return _number; }
		virtual const ImpreciseDate& date() { return _date; }

		virtual Season * season() { return _tvShow->getSeason(_season); }
	private:
		TvShow * _tvShow;
		int _season;
		int _number;
		ImpreciseDate _date;
};

MediaContentLibrary::MediaContentLibrary(Library * library, QSqlDatabase & db) : library(library), MediaPropertyLibrary((LibraryItemRegistrationListener*)library), db(db) { }

int MediaContentLibrary::count(bool libraryOnly) {
	QSqlQuery q(db);
	if(libraryOnly) {
		q.prepare("SELECT COUNT(1) FROM mediaContents WHERE inLibrary=?");
		q.bindValue(0, true);
		if(q.exec() && q.next()) return q.value(0).toInt();
		else return 0;
	} else {
		q.exec("SELECT COUNT(1) FROM mediaContents");
		if(q.next()) return q.value(0).toInt();
		else return 0;
	}
}

MediaContent * MediaContentLibrary::load(QSqlQuery & q) {
	if(q.exec() && q.next()) {
		const int id = q.value(0).toInt();
		SqlMediaContent media(id, q.value(2).toUrl(), q.value(5).toString(), q.value(4).toLongLong());
		const QString type = q.value(3).toString();
		if(type == "song") {
			q.prepare("SELECT * FROM mediaSongs WHERE media=?");
			q.bindValue(0, id);
			if(q.exec() && q.next()) {
				return new SqlSong(media, library, q.value(2).toInt(), library->artists()->get(q.value(4).toInt()));
			}
		} else if(type == "episode") {
			q.prepare("SELECT e.id, s.number, e.number, e.date, s.tvShow FROM mediaTvEpisodes e, tvShowSeasons s WHERE media=? AND e.season=s.id");
			q.bindValue(0, id);
			if(q.exec() && q.next()) {
				const int season = q.value(1).toInt();
				const int number = q.value(2).toInt();
				const QString date = q.value(3).toString();
				//q.prepare("SELECT tvShow FROM tvShowSeasons WHERE id=?");
				//q.bindValue(0, season);
				//if(q.exec() && q.next()) {
					return new SqlEpisode(media, library->tvShows()->get(q.value(4).toInt()), season, number, Library::toDate(date));
				//}
			}
		}
	}

	return NULL;
}

MediaContent* MediaContentLibrary::get(qint64 id) {
	QSqlQuery q(db);
	q.prepare("SELECT * FROM mediaContents WHERE id=?");
	q.bindValue(0, id);

	return load(q);
}

MediaContent* MediaContentLibrary::get(QString uri) {
	QSqlQuery q(db);
	q.prepare("SELECT * FROM mediaContents WHERE uri=?");
	q.bindValue(0, uri);

	return load(q);
}

QList<MediaContent*> MediaContentLibrary::search(MediaContentSearchOptions & options) {
	options.query = removeAccents(options.query.simplified()).toLower();

	QList<int> goodIds;

	QSqlQuery q(db);
	bool hasTextToSearch = !options.query.isEmpty();
	QString query = hasTextToSearch ? "SELECT id, name FROM mediaContents WHERE type=?" : "SELECT id FROM mediaContents WHERE type=?";
	if(options.sort == MediaContentSearchOptions::LastAddedFirst) {
		query += " ORDER BY dateAdded DESC, id DESC";
	}
	//if(options.limit > 0) query += " LIMIT " + QString::number(options.limit);
	q.prepare(query);
	q.bindValue(0, options.type);
	if(q.exec()) {
		while(q.next()) {
			if(hasTextToSearch) {
				QString name = removeAccents(q.value(1).toString().simplified()).toLower();
				if(name.contains(options.query)) {
					goodIds << q.value(0).toInt();
				}
			} else {
				goodIds << q.value(0).toInt();
			}
		}
	} else {
		qDebug() << "MediaContentLibrary:" << query << "failed:" << q.lastError().text();
	}

	QList<MediaContent*> results;
	for(int id : goodIds) results << get(id);

	return results;
}

QString MediaContentLibrary::removeAccents(const QString & string) {
	if (accentedCharacters.isEmpty()) {
		accentedCharacters = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
		baseCharacters << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";
	}

	QString output = "";
	for (int i = 0; i < string.length(); i++) {
		QChar c = string[i];
		int dIndex = accentedCharacters.indexOf(c);
		if (dIndex < 0) {
			output.append(c);
		} else {
			output.append(baseCharacters[dIndex]);
		}
	}

	return output;
}

bool MediaContentLibrary::has(QString uri) {
	QSqlQuery q(db);
	q.prepare("SELECT COUNT(1) FROM mediaContents WHERE uri=?");
	q.bindValue(0, uri);

	if(q.exec() && q.next() && q.value(0).toInt() > 0) return true;
	else return false;
}

int MediaContentLibrary::registerMedia(MediaContent * media, void * from) {
	if(media == NULL) return NULL;

	QMutexLocker locker(&mutex);

	qDebug() << "Media content" << media << "(" << media->name() << "," << media->type() << ") about to be registered";

	db.transaction();
	QSqlQuery q(db);
	q.prepare("INSERT INTO mediaContents (inLibrary, uri, type, length, name, dateAdded) VALUES (?, ?, ?, ?, ?, ?)");
	q.bindValue(0, true);
	q.bindValue(1, media->uri());
	q.bindValue(2, media->type());
	q.bindValue(3, media->length());
	q.bindValue(4, media->name());
	q.bindValue(5, QDateTime::currentMSecsSinceEpoch());
	if(q.exec()) {
		const int id = q.lastInsertId().toInt();

		if(media->type() == "song") {
				Song * song = (Song*) media;
				//q.prepare("INSERT INTO mediaSongs (media, number, album, artist) VALUES (?, ?, ?, ?)");
				q.prepare("INSERT INTO mediaSongs (media, number, artist) VALUES (?, ?, ?)");
				q.bindValue(0, id);
				q.bindValue(1, song->number());
				q.bindValue(2, song->artist() ? song->artist()->id() : QVariant());
				//q.bindValue(3, song->album() ? song->album()->id() : QVariant());
				if(!q.exec()) {
					db.rollback();
					return -1;
				} else {
					q.prepare("DELETE FROM artistsFeaturingMedia WHERE media=?");
					q.bindValue(0, id);
					if(!q.exec()) {
						db.rollback();
						return -1;
					} else {
						q.prepare("INSERT INTO artistsFeaturingMedia (artist, media) VALUES (?, ?)");
						q.bindValue(1, id);
						for(Artist * a : song->featuring()) {
							q.bindValue(0, a->id());
							q.exec();
						}
					}
				}
		} else if(media->type() == "episode") {
				Episode * episode = (Episode*) media;
				q.prepare("INSERT INTO mediaTvEpisodes (media, season, number, date) VALUES (?, ?, ?, ?)");
				q.bindValue(0, id);
				q.bindValue(1, episode->season()->id());
				q.bindValue(2, episode->number());
				q.bindValue(3, Library::toString(episode->date()));
				if(!q.exec()) {
					db.rollback();
					return -1;
				}
		}

		db.commit();
		return id;
	}

	db.rollback();
	return -1;
}

void MediaContentLibrary::deleteAll(void * from) {
	QSqlQuery q(db);
	q.exec("DELETE FROM mediaExtraInfo");
	q.exec("DELETE FROM artistsFeaturingMedia");
	q.exec("DELETE FROM mediaSongs");
	q.exec("DELETE FROM mediaTvEpisodes");
	q.exec("DELETE FROM mediaContents");
}

void MediaContentLibrary::updateMedia(MediaContent * media, qint64 duration, void * from) {
	qDebug() << "Media content" << media << "(" << media->name() << "," << media->type() << ") is getting its duration set to" << duration;
	QSqlQuery q(db);
	q.prepare("UPDATE mediaContents SET length=? WHERE id=?");
	q.bindValue(0, duration);
	q.bindValue(1, media->id());
	if(q.exec()) ((SqlMediaContent*)media)->setLength(duration);
}

bool MediaContentLibrary::setItemInLibrary(MediaContent * m, bool inLibrary) {
	QSqlQuery q(db);
	q.prepare("UPDATE mediaContents SET inLibrary=? WHERE id=?");
	q.bindValue(0, inLibrary);
	q.bindValue(1, m->id());
	return q.exec();
}

bool MediaContentLibrary::setItemName(MediaContent * m, const QString & name) {
	QSqlQuery q(db);
	q.prepare("UPDATE mediaContents SET name=? WHERE id=?");
	q.bindValue(0, name);
	q.bindValue(1, m->id());
	return q.exec();
}

bool MediaContentLibrary::isInLibrary(MediaContent * item) {
	QSqlQuery q(db);
	q.prepare("SELECT inLibrary FROM mediaContents WHERE id=?");
	q.bindValue(0, item->id());
	if(q.exec() && q.next()) return q.value(0).toBool();
	return false;
}
