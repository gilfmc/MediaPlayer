#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore/QSettings>
#include <QtGui/QColor>

#include "Loadable.h"

class LibraryLookupPlacesList;

class LibraryLookupPlace : public QObject {
	friend class LibraryLookupPlacesList;
	Q_OBJECT
	
	Q_PROPERTY(QString path READ path WRITE setPath)
	Q_PROPERTY(bool includeHidden READ includeHidden WRITE setIncludeHidden)
	Q_PROPERTY(bool recursive READ recursive WRITE setRecursive)
	
	public:
		LibraryLookupPlace() {}
		LibraryLookupPlace(QSettings & settings);
		LibraryLookupPlace(QString path);
		LibraryLookupPlace(const LibraryLookupPlace &);
		
		//LibraryLookupPlace& operator=(const LibraryLookupPlace&);
		
		QString path(); void setPath(QString path);
		bool includeHidden(); void setIncludeHidden(bool include);
		bool recursive(); void setRecursive(bool recursive);
		
	private:
		QString _path;
		bool _includeHidden;
		bool _recursive;
		
		void save(QSettings * settings) const;
};
Q_DECLARE_METATYPE(LibraryLookupPlace)

class LibraryLookupPlacesList : public QObject {
	Q_OBJECT
	Q_PROPERTY(int length READ length NOTIFY onLengthChanged)
	Q_PROPERTY(bool hasChangesToSave READ hasChangesToSave NOTIFY onHasChangesToSaveChanged)
	
	public:
		LibraryLookupPlacesList() {}
		LibraryLookupPlacesList(QSettings & settings);
		LibraryLookupPlacesList(const LibraryLookupPlacesList &);
		
		LibraryLookupPlacesList& operator=(const LibraryLookupPlacesList&);
		
		int length();
		
		Q_INVOKABLE
		LibraryLookupPlace* &operator[](int i);
		Q_INVOKABLE
		QObject* at(int i);
		
		Q_INVOKABLE
		QObject * add(QString path);
		Q_INVOKABLE
		void remove(int index);
		void clear();
		
		Q_INVOKABLE
		QString getUserReadablePlaceName(LibraryLookupPlace * place);
		
		Q_INVOKABLE
		void save();
		void save(int setup);
		
		bool hasChangesToSave();
		
		~LibraryLookupPlacesList();

	signals:
		void onLengthChanged();
		void onHasChangesToSaveChanged();

	private:
		QSettings * settings;
		QList<LibraryLookupPlace*> list;
		
		bool changed = false;
};
Q_DECLARE_METATYPE(LibraryLookupPlacesList)

class LibrarySettings : public QObject {
	Q_OBJECT
	
	Q_PROPERTY(LibraryLookupPlacesList * lookupPlaces READ lookupPlaces)
	
	public:
		explicit LibrarySettings(QSettings & settings, QObject* parent = nullptr);
		LibrarySettings(const LibrarySettings & librarySettings);
	
		LibraryLookupPlacesList * lookupPlaces();
	
	private:
		QSettings & settings;
		
		void checkSettings();
};

class Settings : public QObject {
	Q_OBJECT
	
	Q_PROPERTY(int mainWindowWidth READ mainWindowWidth WRITE setMainWindowWidth)
	Q_PROPERTY(int mainWindowHeight READ mainWindowHeight WRITE setMainWindowHeight)

	Q_PROPERTY(QColor primaryColor READ primaryColor WRITE setPrimaryColor)
	Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor)

	Q_PROPERTY(bool librarySidebarOpen READ librarySidebarOpen WRITE setLibrarySidebarOpen)
	Q_PROPERTY(bool playlistSidebarOpen READ playlistSidebarOpen WRITE setPlaylistSidebarOpen)

	Q_PROPERTY(bool restoreState READ restoreState WRITE setRestoreState)
	
	Q_PROPERTY(LibrarySettings * library READ librarySettings)
	
	public:
		explicit Settings(QObject* parent = nullptr);
		
		LibrarySettings * librarySettings();

		int mainWindowWidth();
		int mainWindowHeight();
		void setMainWindowWidth(int width);
		void setMainWindowHeight(int height);

		Q_INVOKABLE
		float dpMultiplier(float defaultMultiplier);
		Q_INVOKABLE
		void setDpMultiplier(float multiplier);

		QColor primaryColor();
		QColor accentColor();
		void setPrimaryColor(QColor);
		void setAccentColor(QColor);
	
		bool librarySidebarOpen();
		void setLibrarySidebarOpen(bool open);
		bool playlistSidebarOpen();
		void setPlaylistSidebarOpen(bool open);

		bool restoreState();
		void setRestoreState(bool restore);

		virtual ~Settings();
		
	private:
		QSettings settings;
		
		LibrarySettings _librarySettings;

		bool _librarySidebarOpen, _playlistSidebarOpen;
};

#endif // SETTINGS_H
