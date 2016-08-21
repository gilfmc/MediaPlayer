#ifndef ALSOKNOWNASELEMENT_H
#define ALSOKNOWNASELEMENT_H

#include <QtCore/QString>

class AlsoKnownAsElement {
	public:
		virtual int size() const = 0;
		virtual const QString & get(int index) const = 0;

		virtual bool contains(const QString & aka) {
			const int len = size();

			for(int i = 0; i < len; i++) {
				if(get(i) == aka) return true;
			}

			return false;
		}

		virtual ~AlsoKnownAsElement() { }
};

#endif // ALSOKNOWNASELEMENT_H
