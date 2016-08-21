#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#include "data/MediaContent.h"
#include "MediaPropertyLibrary.h"

class MediaLibrary : public MediaPropertyLibrary<MediaContent*>
{
	public:
		MediaLibrary();
};

#endif // MEDIALIBRARY_H
