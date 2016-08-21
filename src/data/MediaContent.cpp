#include "MediaContent.h"

MediaContent::MediaContent(int id, QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
				_id(id),
				_uri(uri),
				_name(name),
				_length(length),
				_audio(hasAudio),
				_video(hasVideo)
				{}

MediaContent::MediaContent(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo) :
				_id(-1),
				_uri(uri),
				_name(name),
				_length(length),
				_audio(hasAudio),
				_video(hasVideo)
				{}

MediaContent::MediaContent(const MediaContent & m) :
				_id(m._id),
				_uri(m._uri),
				_name(m._name),
				_length(m._length),
				_audio(m._audio),
				_video(m._video)
				{}

MediaContent::MediaContent(QUrl uri) :
				_id(-1),
				_uri(uri),
				_length(-1),
				_audio(true),
				_video(true)
				{}

int MediaContent::id() const { return _id; }

const QString & MediaContent::name() const { return _name; }

QUrl MediaContent::uri() { return _uri; }

qint64 MediaContent::length() { return _length; }

bool MediaContent::hasAudio() { return _audio; }
bool MediaContent::hasVideo() { return _video; }
