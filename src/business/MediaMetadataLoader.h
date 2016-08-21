#ifndef MEDIAMETADATALOADER_H
#define MEDIAMETADATALOADER_H

#include <QtCore/QRegularExpression>
#include <QtCore/QMimeDatabase>

class MediaMetadataLoader;

#include "MediaPlayerContext.h"
#include "data/MediaContent.h"

class MediaMetadataLoader {
		// QRegularExpression precompiles regular expressions. In order to improve performance (and reduce unnecessary memory usage),
		// let's reuse them all the time.
		class RegexUtils {
			public:
				RegexUtils () :
					groupDelimiters(QRegularExpression("(\\(.+\\))|(\\[.+\\])")),
					delimiter(QRegularExpression("[-\\._]")),
					featuring(QRegularExpression("(.+)[ \\.,\\-'_]+[Ff]([Ee][Aa])?[Tt]([Uu][Rr][Ii][Nn])?[Gg]?[ \\.,\\-'_]*(.+)")),
					delimitedFeaturing(QRegularExpression("[\\.,\\-'_ ]*[Ff]([Ee][Aa])?[Tt]([Uu][Rr][Ii][Nn])?[Gg]?[ \\.,\\-'_]*(.+)")),
					featuringLeftOvers(QRegularExpression("[Ff]([Ee][Aa])?[Tt]([Uu][Rr][Ii][Nn])?[Gg]?[ \\.,\\-'_]*")),
					elementList(QRegularExpression("[&,/]|([ \\.,\\-'_]+and[ \\.,\\-'_]+)|( \\- )")),
					tvShow(QRegularExpression("[Ss](\\d+)[Ee](\\d+)"))
					{ }

				const QRegularExpression groupDelimiters, delimiter, featuring, delimitedFeaturing, featuringLeftOvers, elementList, tvShow;
		};

	public:
		MediaMetadataLoader(MediaPlayerContext * context);
		~MediaMetadataLoader() { }

		MediaContent * load(MediaContent * media);
		MediaContent * load(const QUrl & url);
		MediaContent * load(const QString & url);

	private:
		/*static*/ const RegexUtils regex;

		MediaPlayerContext * context;
		QMimeDatabase mimeDb;

		virtual Artist* registerArtist(ArtistsLibrary * library, QString name);
		void processFeaturing(ArtistsLibrary* artists, QList<Artist*>& list, QString featuring);
};

#endif // MEDIAMETADATALOADER_H
