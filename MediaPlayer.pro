TEMPLATE = app

TARGET = papyrosmediaplayer

QT += sql qml quick widgets multimedia concurrent

CONFIG += link_pkgconfig
linux:!android: {
    QT += dbus
    PKGCONFIG += taglib libffmpegthumbnailer
}
#PKGCONFIG += libmediainfo

CONFIG += c++14

INCLUDEPATH += $$PWD/src

SOURCES += main.cpp \
	src/ui/MainWindow.cpp \
	MediaPlayerContext.cpp \
	MediaContentPlayer.cpp \
    src/data/Album.cpp \
    src/data/AlsoKnownAsElement.cpp \
    src/data/Artist.cpp \
    src/data/ImpreciseDate.cpp \
    src/data/MediaContent.cpp \
    src/data/MediaProperty.cpp \
    src/data/Person.cpp \
	src/data/Song.cpp \
    src/business/PluginManager.cpp \
    src/business/MediaInformationManager.cpp \
    src/data/Playlist.cpp \
#    src/data/TemporaryMediaContent.cpp \
    src/repo/Library.cpp \
    src/repo/MediaPropertyLibrary.cpp \
    src/repo/ArtistsLibrary.cpp \
    src/business/Loadable.cpp \
	src/business/Settings.cpp \
    src/business/LibraryScanner.cpp \
    src/repo/MediaLibrary.cpp \
    src/business/MediaMetadataLoader.cpp \
    src/repo/TvShowsLibrary.cpp \
    src/data/TvShow.cpp \
    src/ui/MediaContentUiProvider.cpp \
    src/repo/MediaContentLibrary.cpp \
    src/data/Episode.cpp \
    src/data/Season.cpp \
    src/ui/MediaListUiProvider.cpp \
    src/ui/PropertyListUiProvider.cpp \
    src/business/MediaImageProvider.cpp \
    src/business/Task.cpp \
    src/business/TaskManager.cpp \
    src/business/Subtitles.cpp \
    src/ui/SearchUi.cpp \
    src/ui/HomeUiProvider.cpp

RESOURCES += src/ui/qml.qrc \
    resources.qrc

lupdate_only {
    SOURCES += src/ui/*.qml \
        src/ui/pages/*.qml \
        src/ui/dialogs/*.qml \
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	src/ui/MainWindow.h \
	MediaPlayerContext.h \
	MediaContentPlayer.h \
	src/data/Album.h \
	src/data/AlsoKnownAsElement.h \
	src/data/Artist.h \
	src/data/ImpreciseDate.h \
	src/data/MediaContent.h \
	src/data/MediaProperty.h \
	src/data/Person.h \
	src/data/Song.h \
	src/business/InformationDetectorPlugin.h \
	src/business/PluginManager.h \
	src/business/MediaInformationManager.h \
	src/data/Playlist.h \
	#    src/data/TemporaryMediaContent.h \
	src/repo/Library.h \
	src/repo/MediaPropertyLibrary.h \
	src/repo/ArtistsLibrary.h \
	src/business/Loadable.h \
	src/business/Settings.h \
	src/business/LibraryScanner.h \
	src/repo/MediaLibrary.h \
	src/business/MediaMetadataLoader.h \
	src/repo/TvShowsLibrary.h \
	src/data/TvShow.h \
	src/ui/MediaContentUiProvider.h \
	src/business/MediaInformationSourcePlugin.h \
	src/repo/MediaContentLibrary.h \
	src/data/Episode.h \
	src/data/Season.h \
	src/ui/MediaListUiProvider.h \
	src/ui/PropertyListUiProvider.h \
	src/business/MediaImageProvider.h \
	src/business/Task.h \
	src/business/TaskManager.h \
	src/business/Subtitles.h \
	src/ui/SearchUi.h \
	src/repo/LibraryItemRegistrationListener.h \
	src/ui/HomeUiProvider.h

linux:!android {
	SOURCES += src/mpris/Mpris.cpp \
		src/mpris/MprisMediaPlayer.cpp \
		src/mpris/MprisMediaPlayerController.cpp
	HEADERS += src/mpris/Mpris.h \
		src/mpris/MprisMediaPlayer.h \
		src/mpris/MprisMediaPlayerController.h
}

# TODO make it work on Android
!linux|android {
	INCLUDEPATH += $$PWD/dependencies/taglib/taglib \
						$$PWD/dependencies/taglib \
						$$PWD/dependencies

	SOURCES +=	dependencies/taglib/toolkit/.cpp \
					dependencies/taglib/asf/.cpp \
					dependencies/taglib/mpeg/.cpp \
					dependencies/taglib/ogg/.cpp \
					dependencies/taglib/ogg/flac/.cpp \
					dependencies/taglib/flac/.cpp \
					dependencies/taglib/mpc/.cpp \
					dependencies/taglib/mp4/.cpp \
					dependencies/taglib/ogg/vorbis/.cpp \
					dependencies/taglib/ogg/speex/.cpp \
					dependencies/taglib/ogg/opus/.cpp \
					dependencies/taglib/mpeg/id3v2/.cpp \
					dependencies/taglib/mpeg/id3v2/frames/.cpp \
					dependencies/taglib/mpeg/id3v1/.cpp \
					dependencies/taglib/mpeg/ape/.cpp \
					dependencies/taglib/mpeg/wavpack/.cpp \
					dependencies/taglib/mpeg/trueaudio/.cpp \
					dependencies/taglib/mpeg/riff/.cpp \
					dependencies/taglib/mpeg/riff/aiff/.cpp \
					dependencies/taglib/mpeg/riff/wav/.cpp \
					dependencies/taglib/mpeg/mod/.cpp \
					dependencies/taglib/mpeg/s3m/.cpp \
					dependencies/taglib/mpeg/it/.cpp \
					dependencies/taglib/mpeg/xm/.cpp
	HEADERS +=	dependencies/taglib/toolkit/.h \
					dependencies/taglib/asf/.h \
					dependencies/taglib/mpeg/.h \
					dependencies/taglib/ogg/.h \
					dependencies/taglib/ogg/flac/.h \
					dependencies/taglib/flac/.h \
					dependencies/taglib/mpc/.h \
					dependencies/taglib/mp4/.h \
					dependencies/taglib/ogg/vorbis/.h \
					dependencies/taglib/ogg/speex/.h \
					dependencies/taglib/ogg/opus/.h \
					dependencies/taglib/mpeg/id3v2/.h \
					dependencies/taglib/mpeg/id3v2/frames/.h \
					dependencies/taglib/mpeg/id3v1/.h \
					dependencies/taglib/mpeg/ape/.h \
					dependencies/taglib/mpeg/wavpack/.h \
					dependencies/taglib/mpeg/trueaudio/.h \
					dependencies/taglib/mpeg/riff/.h \
					dependencies/taglib/mpeg/riff/aiff/.h \
					dependencies/taglib/mpeg/riff/wav/.h \
					dependencies/taglib/mpeg/mod/.h \
					dependencies/taglib/mpeg/s3m/.h \
					dependencies/taglib/mpeg/it/.h \
					dependencies/taglib/mpeg/xm/.h
}

TRANSLATIONS += translations/pt_PT.ts \
                translations/en_US.ts

DISTFILES += \
    icon

ICON = icon.png
RC_ICONS = icon.ico
