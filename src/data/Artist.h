#ifndef ARTIST_H
#define ARTIST_H

#include "MediaContent.h"
#include "AlsoKnownAsElement.h"

class Artist : public MediaContentContainer {
	public:
		virtual AlsoKnownAsElement * alsoKnownAs() = 0;

		virtual const QString & type() const { return _type; }

		virtual const QList<MediaContent*> featuredIn() { return QList<MediaContent*>(); }

	private:
		static const QString _type;
};

#endif // ARTIST_H
