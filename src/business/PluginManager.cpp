#include "PluginManager.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

PluginManager::PluginManager(QObject *parent) : QAbstractListModel(parent)
{
	load();
}

void PluginManager::onLoad() {
	qDebug() << "Looking for plugins...";
	QStringList allPlugins;
	
	QDir pluginsDir(QCoreApplication::applicationDirPath());
	pluginsDir.cd("plugins");
	lookForPlugins(allPlugins, pluginsDir);
	
	lookForPlugins(allPlugins, QDir("/usr/lib64/papyrosmediaplayer/plugins"));
	
	foreach(QString pluginFileName, allPlugins) {
        QPluginLoader * loader = new QPluginLoader(pluginFileName);
		QJsonObject metadata = loader->metaData();
        if(metadata.count() > 0) {
            loaders.append(loader);
            qDebug() << " Loading" << metadata["IID"].toString() << ":" << pluginFileName;

            foreach(QJsonValue imp, metadata["MetaData"].toObject()["implements"].toArray()) {
                if(imp.toString() == "InformationDetector") informationDetectors.append(new Plugin<InformationDetectorPlugin>(loader));
                else if(imp.toString() == "InformationSource") informationSources.append(new Plugin<MediaInformationSourcePlugin>(loader));
            }
        }
	}

	qDebug() << informationDetectorCount() << "information detectors loaded";
	qDebug() << informationSourceCount() << "information sources loaded";
}

void PluginManager::lookForPlugins(QStringList & all, QDir in) {
	foreach(QFileInfo file, in.entryInfoList()) {
		if(file.isSymLink()) {
			if(!all.contains(file.symLinkTarget()))
				all.append(file.symLinkTarget());
		} else if(file.isFile()) {
			if(!all.contains(file.absoluteFilePath()))
				all.append(file.absoluteFilePath());
		}
	}
}

template <class T>
Plugin<T>::Plugin(QPluginLoader * loader, QObject *parent) : loader(loader), QObject(parent) {

}

int PluginManager::informationDetectorCount() const {
	return informationDetectors.count();
}

int PluginManager::informationSourceCount() const {
	return informationSources.count();
}

Plugin<InformationDetectorPlugin> * PluginManager::informationDetector(int index) {
	return informationDetectors[index];
}

Plugin<MediaInformationSourcePlugin> * PluginManager::informationSource(int index) {
	return informationSources[index];
}

template <class T>
Plugin<T>::~Plugin() {
	if(_instance) delete _instance;
}

PluginManager::~PluginManager() {
	foreach(QObject * p, informationDetectors) delete p;

	foreach(QPluginLoader * l, loaders) delete l;
}

QVariant PluginManager::data(const QModelIndex& index, int role) const {
	int infoDetectors = informationDetectorCount();
	int infoSources = informationSourceCount();
	int i = index.row();
	
	if(i == 0) {
		if(role == Qt::UserRole) return 0;
		if(role == Qt::UserRole + 1) return "Information detectors";
		if(role == Qt::UserRole + 2) return QVariant();
	} else if(i <= infoDetectors) {
		if(role == Qt::UserRole) return 1;
		i--;
		if(role == Qt::UserRole + 1) return informationDetectors[i]->name();
		if(role == Qt::UserRole + 2) return informationDetectors[i]->description();
	} else if(i > infoDetectors + 1) {
		if(role == Qt::UserRole) return 1;
		i -= infoDetectors + 2;
		if(role == Qt::UserRole + 1) return informationSources[i]->name();
		if(role == Qt::UserRole + 2) return informationSources[i]->description();
	} else {
		if(role == Qt::UserRole) return 0;
		if(role == Qt::UserRole + 1) return "Media information sources";
		if(role == Qt::UserRole + 2) return QVariant();
	}
	
	return QVariant();
}

QHash< int, QByteArray > PluginManager::roleNames() const {
	QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "type";
    roles[Qt::UserRole + 1] = "name";
    roles[Qt::UserRole + 2] = "description";
    return roles;
}

int PluginManager::rowCount(const QModelIndex& parent) const {
	return informationDetectorCount() + informationSourceCount() + 2;
}
