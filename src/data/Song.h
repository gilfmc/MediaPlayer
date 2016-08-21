#ifndef SONG_H
#define SONG_H

#include <QtCore/QList>

#include "MediaContent.h"
#include "Artist.h"

class Song : public MediaContent {
	public:
		Song(int id, QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo);
		Song(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo);
		Song(int id, QUrl uri, QString name, qint64 length);
		Song(QUrl uri, QString name, qint64 length);
		Song(QUrl uri);
		Song(const MediaContent& m);
		
		virtual int number() = 0;
		virtual Artist * artist() = 0;
		virtual QList<Artist *> & featuring() = 0;

		virtual const QString & type() const { return _type; }

	private:
		static const QString _type;
};

#endif // SONG_H
