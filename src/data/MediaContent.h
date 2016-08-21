#ifndef MEDIACONTENT_H
#define MEDIACONTENT_H

#include <QtCore/QString>
#include <QtCore/QUrl>

#include "MediaProperty.h"

class MediaContent : public MediaProperty {
	public:
		MediaContent(int id, QUrl uri, QString name, qint64 length, bool hasAudio=true, bool hasVideo=true);
		MediaContent(QUrl uri, QString name, qint64 length, bool hasAudio=true, bool hasVideo=true);
		MediaContent(const MediaContent & m);
		MediaContent(QUrl uri);

		virtual int id() const;

		virtual const QString & name() const;

		virtual QUrl uri();

		virtual qint64 length();

		virtual bool hasAudio();
		virtual bool hasVideo();
		//virtual bool isNew() { return false; }

		virtual ~MediaContent() { }

	protected:
		int _id;
		QUrl _uri;
		QString _name;
		qint64 _length;
		bool _audio, _video;
};

class MediaContentContainer : public MediaProperty {
	public:
		virtual const QList<MediaContent*> getMediaContents() { return QList<MediaContent*>(); }
};

#endif // MEDIACONTENT_H
