#ifndef TEMPORARYMEDIACONTENT_H
#define TEMPORARYMEDIACONTENT_H

#include <QtMultimedia/QMediaContent>
#include <QtMultimedia/QMediaPlayer>

class TemporaryMediaContent;

#include "data/Artist.h"
#include "data/TvShow.h"
#include "data/Song.h"
#include "Playlist.h"

#include "repo/ArtistsLibrary.h"

class TemporaryArtist : public Artist {
	public:
		TemporaryArtist(QString name) : _name(name) {}
		virtual const QString & name() const { return _name; }
		virtual AlsoKnownAsElement * alsoKnownAs() { return NULL; }
		int id() const { return -1; }
	private:
		const QString _name;
};

class TemporaryTvShow : public TvShow {
	public:
		TemporaryTvShow(QString name) : _name(name) {}
		virtual const QString & name() const { return _name; }
		virtual AlsoKnownAsElement * alsoKnownAs() { return NULL; }
		int id() const { return -1; }
		virtual Season * getSeason(int number) { return NULL; }
		virtual const QList<Season*> & seasons() { return _seasons; }
	private:
		const QString _name;
		QList<Season*> _seasons;
};

//class TemporaryMediaContent : public QObject, public Song {
//	Q_OBJECT
	
//	public:
//		TemporaryMediaContent(QUrl uri);

//		int id() const { return -1; }
		
//		//virtual QUrl uri();
//		//virtual const QString & name() const;
//		//virtual qint64 length();
		
//		//virtual bool hasAudio() { return true; }
//		//virtual bool hasVideo() { return true; }
//		//virtual bool isNew() { return false; }
		
//		virtual int number();
//		virtual Artist* artist();
//		virtual QList< Artist* > & featuring();
		
//		void loadData(QMediaContent mediaContent, Playlist* playlist);
		
//		void setName(const QString & name);
//		void setLength(qint64 length);
//		//QString _name;
//		//qint64 _length;
		
//		int _number;
//		Artist * _artist = NULL;
//		QList<Artist*> _featuring;
		
//	private:
//		//const QUrl _uri;
		
//		QMediaPlayer * player;
//		Playlist * playlist;
		
//		Artist* registerArtist(QString name, Playlist * playlist);
//		void processFeaturing(ArtistsLibrary* artists, QString featuring);
		
//	private slots:
//		void loadMediaData(QMediaPlayer::MediaStatus);
//};

#endif // TEMPORARYMEDIACONTENT_H
