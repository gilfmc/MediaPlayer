#ifndef HOMEUIPROVIDER_H
#define HOMEUIPROVIDER_H

#include <QtCore/QAbstractListModel>

#include "MediaPlayerContext.h"

class HomePanelUiProvider : public QObject {
	Q_OBJECT

	public:
		HomePanelUiProvider(MediaPlayerContext & context, QObject * parent);

		virtual QString qmlSource() const = 0;

	protected:
		Library * library();

	private:
		MediaPlayerContext & context;
};

class HomePanelItem {
	public:
		HomePanelItem(const QString & name, const QString & description);

		bool enabled();
		void enable();
		void disable();
		void setEnabled(bool enabled);

		QString name();
		QString description();

		HomePanelUiProvider * uiProvider();

	private:
		bool _enabled;
		QString _name;
		QString _description;
};

class HomeUiProvider : public QObject {
	Q_OBJECT
	Q_PROPERTY(int widgetCount READ widgetCount NOTIFY widgetCountChanged)

	public:
		HomeUiProvider(MediaPlayerContext & context, QQmlApplicationEngine & engine, QObject * parent);

		Q_INVOKABLE
		QQuickItem * widgetQmlItemAt(int index);

		int widgetCount();

		QQmlComponent * makeComponent(const QString & path) {
			return new QQmlComponent(&engine, QUrl(path), this);
		}

		void letUserJoinOldPlaylist() {
			QObject * item = engine.rootObjects()[0]->findChild<QObject*>("uiState");
			if(item) {
				QMetaObject::invokeMethod(item, "letUserJoinOldPlaylist", Qt::AutoConnection);
			}
		}

	signals:
		void widgetCountChanged(int);

	private:
		MediaPlayerContext & context;
		QQmlApplicationEngine & engine;
		int _widgetCount;
		char * widgets;
};

class HomePanelListWidget : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(int itemLimit READ itemLimit WRITE setItemLimit)

	public:
		HomePanelListWidget(QQuickItem * item) : item(item) { }

		int itemLimit() { return limit; }
		virtual void setItemLimit(int itemLimit) = 0;

		Q_INVOKABLE
		void notifyCardDismissed() {
			onCardDismissed();
			item->deleteLater();
		}

		virtual void onCardDismissed() = 0;

	public slots:
		virtual void onMediaContentClick(int index, bool play) {
			Q_UNUSED(index); Q_UNUSED(play);
		}

	protected:
		int limit = -1;

	private:
		QQuickItem * item;
};

class HomePanelGridWidget : public QAbstractListModel {
	Q_OBJECT
	Q_PROPERTY(QQmlComponent * component READ component)

	public:
		HomePanelGridWidget(const HomePanelGridWidget & from) : provider(from.provider), item(from.item) {}
		HomePanelGridWidget(HomeUiProvider & provider, QQuickItem * item) : provider(provider), item(item) {}

		virtual QQmlComponent * component() {
			return provider.makeComponent("Material.ListItems.Standard");
		}

		Q_INVOKABLE
		void notifyCardDismissed() {
			onCardDismissed();
			item->deleteLater();
		}

		Q_INVOKABLE
		virtual void onItemClick(int) = 0;

		virtual void onCardDismissed() = 0;

	protected:
		HomeUiProvider & provider;

	private:
		QQuickItem * item;
};

Q_DECLARE_METATYPE(HomePanelGridWidget*)

class HomePanelMessageWidget : public QObject {
	Q_OBJECT

	public:
		HomePanelMessageWidget(HomeUiProvider & provider, const QString & title = 0, const QString & message = 0) {
			QQmlComponent * component = provider.makeComponent("qrc:/components/WidgetBaseMessage.qml");
			QObject * object = component->create();
			object->setProperty("title", title);
			object->setProperty("message", message);
			object->setProperty("model", QVariant::fromValue(this));
			item = qobject_cast<QQuickItem*>(object);
		}

		Q_INVOKABLE
		void notifyCardDismissed() {
			onCardDismissed();
			item->deleteLater();
		}

		virtual void onCardDismissed() = 0;

		QQuickItem * widgetItem() { return item; }

	protected:
		void setMessage(const QString & message) {
			item->setProperty("message", message);
		}

	private:
		QQuickItem * item;
};

#endif // HOMEUIPROVIDER_H
