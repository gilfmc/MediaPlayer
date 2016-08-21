#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T02:08:44
#
#-------------------------------------------------

QT       += network
QT       -= gui

TARGET = OMDbInformationDetectorPlugin
TEMPLATE = lib

DEFINES += OMDB_LIBRARY

SOURCES += OMDbInformationDetectorPlugin.cpp\
		../../../src/data/ImpreciseDate.cpp

HEADERS += OMDbInformationDetectorPlugin.h\
		omdb_global.h\
		../../../src/business/InformationDetectorPlugin.h\
		../../../src/business/MediaInformationSourcePlugin.h\
        ../../../src/business/PluginNetworkAccessHelper.h\
		../../../src/data/ImpreciseDate.h

INCLUDEPATH += $$PWD/../../../src

unix {
	target.path = /usr/lib64/papyrosmediaplayer/plugins
    INSTALLS += target
}

DISTFILES += \
	manifest.json
