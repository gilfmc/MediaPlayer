#ifndef SUBTITLES_H
#define SUBTITLES_H

#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTime>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>

typedef struct Subtitle {
		int start, end;
		QString text;

		Subtitle() {}
		Subtitle(int start, int end, QString text) : start(start), end(end), text(text) {}

		virtual ~Subtitle() {}
} Subtitle;

class Subtitles {
	public:
		Subtitles();
		Subtitles(int count, Subtitle * subtitles);

		void moveToTime(int time) {
			int i;
			for(i = 0; i < count; i++) {
				if(subtitles[i].start > time || subtitles[i].end > time) {
					break;
				}
			}
			cursor = i-1;
			if(cursor < 0) cursor = 0;
		}

		void moveToNext() {
			cursor++;
		}

		bool hasNext() {
			return cursor < count;
		}

		Subtitle * getAtCursor() {
			return &subtitles[cursor];
		}

		Subtitle * operator[](std::size_t i) {
			if(i < count) return &subtitles[i];
			return NULL;
		}

		virtual ~Subtitles();
	private:
		int count = 0;
		int cursor = 0;
		Subtitle * subtitles = NULL;
};

class SrtSubtitleLoader {
	public:
		SrtSubtitleLoader(QString url);

		Subtitles * loadSubtitles();

	private:
		QString url;
};

#endif // SUBTITLES_H
