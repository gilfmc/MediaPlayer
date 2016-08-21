#ifndef EPISODE_H
#define EPISODE_H

class Episode;

#include "MediaContent.h"
#include "ImpreciseDate.h"
#include "Season.h"

class Episode : public MediaContent
{
	public:
		Episode(int id, QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo);
		Episode(QUrl uri, QString name, qint64 length, bool hasAudio, bool hasVideo);
		Episode(int id, QUrl uri, QString name, qint64 length);
		Episode(QUrl uri, QString name, qint64 length);
		Episode(QUrl uri);
		Episode(const MediaContent& m);

		virtual int number() = 0;
		virtual const ImpreciseDate & date() = 0;

		virtual Season * season() = 0;

		virtual const QString & type() const { return _type; }

	private:
		static const QString _type;
};

#endif // EPISODE_H
