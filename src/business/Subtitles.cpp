#include "Subtitles.h"

#include <QtCore/QRegularExpression>

Subtitles::Subtitles() { }

Subtitles::Subtitles(int count, Subtitle * subtitles) : count(count), subtitles(subtitles) { }

Subtitles::~Subtitles() {
	if(subtitles) delete[] subtitles;
}

Subtitles * SrtSubtitleLoader::loadSubtitles() {
	QFile file(url);
	if(!file.open(QIODevice::ReadOnly)) return NULL;

	QTextStream in(&file);
	in.setCodec(QTextCodec::codecForName("ISO-8859-15"));
	in.setAutoDetectUnicode(true);

	QList<Subtitle> subtitles;

	bool ok;
	char part = 0;

	int start, end;
	QString text = "";

	QRegularExpression fontSize("<font (.* )?size=\"?(\\d*(\\.\\d+)?)\"?.*>");
	QRegularExpression fontColor("<font (.* )?color=\"?((\\d|[a-f]){3,6})\"?.*>");
	QRegularExpression specialGroups("{\\\\.*}");

	while(!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if(line.length() == 0) {
			subtitles.append(Subtitle(start, end, text));
			text = "";
			part = 0;
			continue;
		}

		if(part == 0) {
			line.toInt(&ok);
			if(ok) part = 1;
		} else if(part == 1) {
			QStringList times = line.split("-->");
			if(times.length() == 2) {
				start = QTime::fromString(times.at(0).trimmed(), "HH:mm:ss,zzz").msecsSinceStartOfDay();
				end = QTime::fromString(times.at(1).trimmed(), "HH:mm:ss,zzz").msecsSinceStartOfDay();
				part = 2;
			}
		} else {
			line = line.trimmed().replace("\\N", "\\n").replace("\\h", "\302\240");

			// TODO support those groups instead
			line.remove(specialGroups);

			// quick fixes for font size
			QRegularExpressionMatch fontMatch = fontSize.match(line);
			if(fontMatch.hasMatch()) {
				bool sizeOk;
				float size = fontMatch.captured(2).toFloat(&sizeOk);
				if(sizeOk) {
					line = line.replace(fontMatch.capturedStart(2), fontMatch.capturedLength(2), QString::number(size/3));
				}
			}
			// fix wrong color syntax
			fontMatch = fontColor.match(line);
			if(fontMatch.hasMatch()) {
				line = line.insert(fontMatch.capturedStart(2), "#");
			}
			if(text.length() == 0) text = line;
			else text += "<br>" + line;
		}
	}
	file.close();

	Subtitle * result = new Subtitle[subtitles.size()];
	int i = 0;
	for(const Subtitle & s : subtitles) result[i++] = s;

	return new Subtitles(subtitles.size(), result);
}

SrtSubtitleLoader::SrtSubtitleLoader(QString url) : url(url) {
}
