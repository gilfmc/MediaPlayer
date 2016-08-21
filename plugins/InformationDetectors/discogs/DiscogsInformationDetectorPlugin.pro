#-------------------------------------------------
#
# Project created by QtCreator 2016-04-11T06:09:03
#
#-------------------------------------------------

QT       += network
QT       -= gui

TARGET = DiscogsInformationDetectorPlugin
TEMPLATE = lib

DEFINES += DISCOGSINFORMATIONDETECTORPLUGIN_LIBRARY

SOURCES += DiscogsInformationDetectorPlugin.cpp\
		../../../src/data/ImpreciseDate.cpp

HEADERS += DiscogsInformationDetectorPlugin.h\
		discogsinformationdetectorplugin_global.h\
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
