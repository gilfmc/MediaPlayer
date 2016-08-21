#include "Settings.h"

#include <QtCore/QDebug>
#include <QtCore/qstandardpaths.h>
#include <QtCore/QUrl>

Settings::Settings(QObject* parent): QObject(parent), _librarySettings(settings, this) {
	_librarySidebarOpen = settings.value("mainWindow/showLibrary", true).toBool();
	_playlistSidebarOpen = settings.value("mainWindow/showPlaylist", true).toBool();
}

int Settings::mainWindowHeight() { return settings.value("mainWindow/height", 580).toInt(); }
int Settings::mainWindowWidth() { return settings.value("mainWindow/width", 1024).toInt(); }

void Settings::setMainWindowHeight(int height) { settings.setValue("mainWindow/height", height); }
void Settings::setMainWindowWidth(int width) { settings.setValue("mainWindow/width", width); }

float Settings::dpMultiplier(float defaultMultiplier) { return settings.value("dpMultiplier", defaultMultiplier).toFloat(); }
void Settings::setDpMultiplier(float multiplier) { settings.setValue("dpMultiplier", multiplier); }

QColor Settings::primaryColor() { return settings.value("theme/primaryColor", 0xffFF9800).toInt(); }
QColor Settings::accentColor() { return settings.value("theme/accentColor", 0xffFF9100).toInt(); }

void Settings::setPrimaryColor(QColor color) { settings.setValue("theme/primaryColor", color.rgb()); }
void Settings::setAccentColor(QColor color) { settings.setValue("theme/accentColor", color.rgb()); }

bool Settings::librarySidebarOpen() { return _librarySidebarOpen; }
bool Settings::playlistSidebarOpen() { return _playlistSidebarOpen; }

void Settings::setLibrarySidebarOpen(bool open) { _librarySidebarOpen = open; }
void Settings::setPlaylistSidebarOpen(bool open) { _playlistSidebarOpen = open; }

bool Settings::restoreState() { return settings.value("restoreState", true).toBool(); }
void Settings::setRestoreState(bool restore) { settings.setValue("restoreState", restore); }

Settings::~Settings() {
	settings.setValue("mainWindow/showLibrary", _librarySidebarOpen);
	settings.setValue("mainWindow/showPlaylist", _playlistSidebarOpen);
}

LibrarySettings * Settings::librarySettings() {
	return &_librarySettings;
}

LibrarySettings::LibrarySettings(QSettings& settings, QObject* parent): QObject(parent), settings(settings) { checkSettings(); }

LibrarySettings::LibrarySettings(const LibrarySettings& librarySettings): QObject(librarySettings.parent()), settings(librarySettings.settings) { checkSettings(); }

void LibrarySettings::checkSettings() {
	int setup = settings.value("library/lookupPlaces/setup", -1).toInt();
	switch(setup) {
		case -1:	// never set
			LibraryLookupPlacesList * places = lookupPlaces();
			QStandardPaths::StandardLocation locations[] = {QStandardPaths::MusicLocation,
															QStandardPaths::MoviesLocation,
															QStandardPaths::PicturesLocation,
															QStandardPaths::DesktopLocation,
															QStandardPaths::DownloadLocation,
															QStandardPaths::DocumentsLocation};
			const int len = 6;
			for(int i = 0; i < len; i++) {
				QStringList paths = QStandardPaths::standardLocations(locations[i]);
				if(paths.length() > 0) places->add(paths[0]);
			}
			places->save(+1);
			return;
	}
}

LibraryLookupPlacesList * LibrarySettings::lookupPlaces() {
	return new LibraryLookupPlacesList(settings);
}

LibraryLookupPlacesList::LibraryLookupPlacesList(QSettings& settings) : settings(&settings), changed(false) {
	const int size = settings.beginReadArray("library/lookupPlaces");
	//qDebug() << "Loading" << size << "library lookup places";
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		list.append(new LibraryLookupPlace(settings));
	}
	settings.endArray();
}

LibraryLookupPlacesList::LibraryLookupPlacesList(const LibraryLookupPlacesList & places) : list(places.list), settings(places.settings), changed(places.changed) { }

LibraryLookupPlacesList & LibraryLookupPlacesList::operator=(const LibraryLookupPlacesList & from) {
	settings = from.settings;
	changed = from.changed;
	list.clear();
	list.append(from.list);
	return *this;
}

void LibraryLookupPlacesList::save() {
	save(0);
}

void LibraryLookupPlacesList::save(int setup) {
	if(!changed) return;
	
	const int size = list.count();
	qDebug() << "Saving" << size << "library lookup places";
	settings->setValue("library/lookupPlaces/setup", setup);
	settings->beginWriteArray("library/lookupPlaces", size);
	for (int i = 0; i < size; ++i) {
		settings->setArrayIndex(i);
		list.at(i)->save(settings);
	}
	settings->endArray();
	settings->sync();
	changed = false;
	emit onHasChangesToSaveChanged();
}

int LibraryLookupPlacesList::length() { return list.length(); }

LibraryLookupPlace*& LibraryLookupPlacesList::operator[](int i) { return list[i]; }
QObject* LibraryLookupPlacesList::at(int i) { return list.at(i); }

QObject* LibraryLookupPlacesList::add(QString path) {
	changed = true;
	emit onHasChangesToSaveChanged();
	LibraryLookupPlace * place = new LibraryLookupPlace(QUrl(path).path());
	list.append(place);
	emit onLengthChanged();
	return place;
}

void LibraryLookupPlacesList::remove(int index) {
	changed = true;
	emit onHasChangesToSaveChanged();
	list.removeAt(index);
	emit onLengthChanged();
}

void LibraryLookupPlacesList::clear() {
	changed = true;
	emit onHasChangesToSaveChanged();
	list.clear();
	emit onLengthChanged();
}

bool LibraryLookupPlacesList::hasChangesToSave() {
	return changed;
}

QString LibraryLookupPlacesList::getUserReadablePlaceName(LibraryLookupPlace* place) {
	if(place == NULL) return QString();
	
	QUrl path(place->_path);
	QString uPath = path.path();
	QStandardPaths::StandardLocation locations[] = {QStandardPaths::MusicLocation,
													QStandardPaths::MoviesLocation,
													QStandardPaths::PicturesLocation,
													QStandardPaths::DesktopLocation,
													QStandardPaths::DownloadLocation,
													QStandardPaths::DocumentsLocation,
													QStandardPaths::HomeLocation};
	const int len = 7;
	for(int i = 0; i < len; i++) {
		QStringList paths = QStandardPaths::standardLocations(locations[i]);
		if(paths.length() > 0) {
			QUrl sPath(paths.first());
			if(uPath == sPath.path()) return tr("%1 folder")
												.arg(QStandardPaths::displayName(locations[i]));
		}
	}
	
	return path.scheme() == "file" ? path.path() : path.toString();
}

LibraryLookupPlacesList::~LibraryLookupPlacesList() {
	qDebug() << "Deleting this LibraryLookupPlacesList...";
	const int len = list.count();
	for(int i = 0; i < len; i++) delete list.at(i);
}

LibraryLookupPlace::LibraryLookupPlace(QSettings& settings) : QObject(),
	_path(settings.value("path").toString()), _includeHidden(settings.value("includeHidden").toBool()), _recursive(settings.value("recursive").toBool()) { }

LibraryLookupPlace::LibraryLookupPlace(QString path) : QObject(), _path(path), _includeHidden(false), _recursive(true) { }

LibraryLookupPlace::LibraryLookupPlace(const LibraryLookupPlace& llp) : QObject(llp.parent()),
	_path(llp._path), _includeHidden(llp._includeHidden), _recursive(llp._recursive) { }

// LibraryLookupPlace& LibraryLookupPlace::operator=(const LibraryLookupPlace& place) {
// 	
// }

void LibraryLookupPlace::save(QSettings* settings) const {
	settings->setValue("path", _path);
	settings->setValue("includeHidden", _includeHidden);
	settings->setValue("recursive", _recursive);
}

QString LibraryLookupPlace::path() {
	return _path;
}
void LibraryLookupPlace::setPath(QString path) {
	_path = path;
}

bool LibraryLookupPlace::includeHidden() {
	return _includeHidden;
}
void LibraryLookupPlace::setIncludeHidden(bool include) {
	_includeHidden = include;
}

bool LibraryLookupPlace::recursive() {
	return _recursive;
}
void LibraryLookupPlace::setRecursive(bool recursive) {
	_recursive = recursive;
}
