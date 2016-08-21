#include "Song.h"

const QString Song::_type = "song";

Song::Song(int id, QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
	MediaContent(id, uri, name, length, hasAudio, hasVideo) { }

Song::Song(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
	MediaContent(uri, name, length, hasAudio, hasVideo) { }

Song::Song(int id, QUrl uri, QString name, qint64 length) :
	Song(id, uri, name, length, true, true) { }

Song::Song(QUrl uri, QString name, qint64 length) :
	Song(uri, name, length, true, true) { }

Song::Song(QUrl uri) :
	MediaContent(uri) { }

Song::Song(const MediaContent& m) :
	MediaContent(m) { }
