#ifndef ALBUM_H
#define ALBUM_H

#include "MediaProperty.h"
#include "ImpreciseDate.h"

class Album : public MediaProperty
{
	public:
		ImpreciseDate date();

		virtual const QString & type() const { return _type; }

	private:
		static const QString _type;
};

#endif // ALBUM_H
