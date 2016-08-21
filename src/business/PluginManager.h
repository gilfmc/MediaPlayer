#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QtCore/QDebug>
#include <QtCore/QPluginLoader>

#include <QtCore/QAbstractListModel>
#include <QtCore/QDir>

#include "InformationDetectorPlugin.h"
#include "MediaInformationSourcePlugin.h"
#include "Loadable.h"

template<class T>
class Plugin : public QObject {
	public:
		explicit Plugin(QPluginLoader * loader, QObject * parent = 0);

		bool isEnabled() {
			return true;
		}
		
		bool isInstanciated() {
			return _instance;
		}

		T * instance() {
			if(_instance) return _instance;

			if(loader->load()) {
				_instance = qobject_cast<T*>(loader->instance());

				return _instance;
			} else {
				qDebug() << "Failed to load plugin: " << loader->errorString();
			}

			return NULL;
		}

		QString id() {
			return loader->metaData()["IID"].toString();
		}
		QString name() {
			return loader->metaData()["MetaData"].toObject()["name"].toString();
		}
		QString description() {
			return loader->metaData()["MetaData"].toObject()["description"].toString();
		}

		virtual ~Plugin();

	private:
		T * _instance = NULL;
		QPluginLoader * loader;
};

class PluginManager : public QAbstractListModel, Loadable {
		Q_OBJECT

	public:
		explicit PluginManager(QObject *parent = 0);

		int informationDetectorCount() const;
		Plugin<InformationDetectorPlugin> * informationDetector(int index);
		int informationSourceCount() const;
		Plugin<MediaInformationSourcePlugin> * informationSource(int index);

		virtual ~PluginManager();
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;

	private:
		QList<Plugin<InformationDetectorPlugin>*> informationDetectors;
		QList<Plugin<MediaInformationSourcePlugin>*> informationSources;
		QList<QPluginLoader*> loaders;
		
		void lookForPlugins(QStringList & all, QDir in);
		
		virtual void onLoad();

	signals:

	public slots:
};

#endif // PLUGINMANAGER_H
