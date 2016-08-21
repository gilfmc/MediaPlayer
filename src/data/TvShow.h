#ifndef TVSHOW_H
#define TVSHOW_H

#include "MediaContent.h"
#include "AlsoKnownAsElement.h"
#include "data/Season.h"

class TvShow : public MediaContentContainer {
	public:
		virtual AlsoKnownAsElement * alsoKnownAs() = 0;

		virtual const QString & type() const { return _type; }

		virtual Season * getSeason(int number) = 0;
		virtual const QList<Season*> & seasons() = 0;

	private:
		static const QString _type;
};

#endif // TVSHOW_H
