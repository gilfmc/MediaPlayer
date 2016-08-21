#ifndef SEASON_H
#define SEASON_H

class Season;

#include "MediaContent.h"
#include "Episode.h"

class Season : public MediaContentContainer {
	public:
		virtual int number() = 0;

		virtual const QString & type() const { return _type; }

		virtual const MediaProperty * parent() = 0;

		virtual int episodeCount() {
			QList<Episode*> episodes = this->episodes();
			const int count = episodes.length();
			for(Episode * e : episodes) delete e;
			return count;
		}
		virtual const QList<Episode*> episodes() = 0;

		virtual const QList<MediaContent*> getMediaContents() {
			QList<MediaContent*> list;
			for(Episode * e : episodes()) list.append((MediaContent*)e);
			return list;
		}

	private:
		static const QString _type;
};

#endif // SEASON_H
