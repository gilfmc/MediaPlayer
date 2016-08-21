#include "Episode.h"

const QString Episode::_type = "episode";

Episode::Episode(int id, QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
	MediaContent(id, uri, name, length, hasAudio, hasVideo) { }

Episode::Episode(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
	MediaContent(uri, name, length, hasAudio, hasVideo) { }

Episode::Episode(int id, QUrl uri, QString name, qint64 length) :
	Episode(id, uri, name, length, true, true) { }

Episode::Episode(QUrl uri, QString name, qint64 length) :
	Episode(uri, name, length, true, true) { }

Episode::Episode(QUrl uri) :
	MediaContent(uri) { }

Episode::Episode(const MediaContent& m) :
	MediaContent(m) { }
