#include "HomeUiProvider.h"

#include <QtConcurrent/QtConcurrent>

class LatestEpisodesWidget : public HomePanelListWidget {
	public:
		LatestEpisodesWidget(MediaPlayerContext * context, QQuickItem * item) : HomePanelListWidget(item), context(context) {
			item->setProperty("title", QObject::tr("Newly added episodes"));
			item->setProperty("model", QVariant::fromValue(this));

			QTimer::singleShot(250, [this]() {
				QFutureWatcher<void> * watcher = new QFutureWatcher<void>();
				connect(watcher, &QFutureWatcher<void>::finished, [this, watcher]() {
					beginInsertRows(QModelIndex(), 0, rowCount()-1);
					endInsertRows();
					watcher->deleteLater();
				});
				watcher->setFuture(QtConcurrent::run([this]() {
					MediaContentSearchOptions options;
					options.type = "episode";
					options.sort = MediaContentSearchOptions::LastAddedFirst;
					episodes = this->context->library()->mediaContents()->search(options).mid(0, 16);
				}));
			});
		}

		void onCardDismissed() {
		}

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
			int i = index.row();

			role -= Qt::UserRole;
			switch(role) {
				case 0: return episodes.at(i)->id();
				case 1: return ((Episode*)episodes.at(i))->number();
				case 2: {
					Episode * e = (Episode*)episodes.at(i);
					if(e->name().isEmpty()) return QObject::tr("Episode %1").arg(e->number());
					else return QString::number(e->number()) + " - " + e->name();
				}
				case 3: {
					Season * s = ((Episode*)episodes.at(i))->season();
					if(s) {
						if(s->name().isEmpty()) return QObject::tr("Season %1").arg(s->number());
						else return s->name();
					}
					return "";
				}
				case 4: return context->formatLength(episodes.at(i));
				case 5: {
					Season * s = ((Episode*)episodes.at(i))->season();
					if(s) {
						const MediaProperty * ts = s->parent();
						if(ts) return ts->name();
					}
					return "";
				}
			}

			return QVariant();
		}

		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const {
			const int realCount = episodes.length();
			if(limit >= 0 && realCount > limit) return limit;
			else return realCount;
		}

		virtual QHash<int, QByteArray> roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "id";
			roles[Qt::UserRole + 1] = "number";
			roles[Qt::UserRole + 2] = "title";
			roles[Qt::UserRole + 3] = "description";
			roles[Qt::UserRole + 4] = "length";
			roles[Qt::UserRole + 5] = "date";
			return roles;
		}

		void setItemLimit(int itemLimit) {
			const int before = rowCount();
			limit = itemLimit;
			const int after = rowCount();
			if(before > after) {
				beginRemoveRows(QModelIndex(), after, before-1);
				endRemoveRows();
			} else if(after > before) {
				beginInsertRows(QModelIndex(), before, after-1);
				endInsertRows();
			}
		}

		~LatestEpisodesWidget() {
			for(MediaContent * m : episodes) delete m;
		}

	public slots:
		virtual void onMediaContentClick(int i, bool play) {
			Playlist * playlist = context->playlist();
			if(playlist) {
				int indexToPlay = play ? playlist->mediaCount() : 0;
				// TODO: passing the MediaContent itself will crash the app if done twice because it gets modified
				playlist->addMedia(episodes.at(i)->id());
				if(play) playlist->play(indexToPlay);
			}
		}

	private:
		QList<MediaContent*> episodes;
		MediaPlayerContext * context;
};

class AutomaticPlaylistsWidget : public HomePanelGridWidget {
	public:
		AutomaticPlaylistsWidget(HomeUiProvider & provider, MediaPlayerContext * context, QQuickItem * item) : HomePanelGridWidget(provider, item), context(context) {
			item->setProperty("title", QObject::tr("Listen to/watch"));
			item->setProperty("model", QVariant::fromValue(this));
		}

		void onItemClick(int index) {
			switch (index) {
				case 0: {
					MediaContentSearchOptions o;
					o.type = "song";
					o.sort = MediaContentSearchOptions::NameAscending;
					provider.letUserJoinOldPlaylist();
					context->playlist()->clear();
					context->playlist()->addMedia(context->library()->mediaContents()->search(o));
					context->play();
					break;
				}
				case 1: {
					MediaContentSearchOptions o;
					o.type = "song";
					provider.letUserJoinOldPlaylist();
					QList<MediaContent*> media = context->library()->mediaContents()->search(o);
					std::random_device rd;
					std::mt19937 rng(rd());
					std::shuffle(media.begin(), media.end(), rng);
					long wantedLength = 3600000; // 1 hour
					int i = 0;
					for(MediaContent * m : media) {
						if(m->length() == -1) wantedLength -= 180000;
						else wantedLength -= m->length();

						i++;
						if(wantedLength <= 0) break;
					}
					QList<MediaContent*> mediaToAdd = media.mid(0, i);
					const int len = media.length();
					i++;
					for(;i < len; i++) {
						delete media[i];
					}
					context->playlist()->clear();
					context->playlist()->addMedia(mediaToAdd);
					context->play();
					break;
				}
				case 2: {
					QList<TvShow*> tvShows = context->library()->tvShows()->naturallySortedList();
					const int len = tvShows.length();
					if(len > 0) {
						TvShow * tvShow = tvShows[qrand() % len];
						if(tvShow) {
							provider.letUserJoinOldPlaylist();
							Playlist * playlist = context->playlist();
							playlist->clear();
							playlist->setExtraInfoProvider(PlaylistExtraInfoProvider::makeProvider(tvShow));
							playlist->addMedia(tvShow->getMediaContents());
							context->play();
						}
					}
				}
			}
		}

		void onCardDismissed() {}

		virtual int rowCount(const QModelIndex & parent) const {
			return 3;
		}

		virtual int columnCount(const QModelIndex & parent) const {
			return 3;
		}

		virtual QHash<int, QByteArray> roleNames() const {
			QHash<int, QByteArray> roles;
			roles[Qt::UserRole] = "_text";
			roles[Qt::UserRole+1] = "_icon";
			roles[Qt::UserRole+2] = "columnCount";
			roles[Qt::UserRole+3] = "_listener";
			return roles;
		}

		virtual QQmlComponent * component() {
			return provider.makeComponent("qrc:/components/BigButton.qml");
		}

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
			int i = index.row();

			role -= Qt::UserRole;
			switch(role) {
				case 0: return i == 0 ? QObject::tr("All songs") : i == 1 ? QObject::tr("Random songs") : QObject::tr("Binge watch");
				case 1: return i == 2 ? "hardware/tv" : "av/library_music";
				case 2: return 3;
				case 3: return QVariant::fromValue((HomePanelGridWidget*)this);
//				case 0: return episodes.at(i)->id();
//				case 1: return ((Episode*)episodes.at(i))->number();
//				case 2: {
//					Episode * e = (Episode*)episodes.at(i);
//					if(e->name().isEmpty()) return QObject::tr("Episode %1").arg(e->number());
//					else return QString::number(e->number()) + " - " + e->name();
//				}
//				case 3: {
//					Season * s = ((Episode*)episodes.at(i))->season();
//					if(s) {
//						if(s->name().isEmpty()) return QObject::tr("Season %1").arg(s->number());
//						else return s->name();
//					}
//					return "";
//				}
//				case 4: return context->formatLength(episodes.at(i));
//				case 5: {
//					Season * s = ((Episode*)episodes.at(i))->season();
//					if(s) {
//						const MediaProperty * ts = s->parent();
//						if(ts) return ts->name();
//					}
//					return "";
//				}
			}

			return QVariant();
		}

	private:
		MediaPlayerContext * context;
};

class WelcomeMessageWidget : public HomePanelMessageWidget {
	public:
		WelcomeMessageWidget(HomeUiProvider & provider) : HomePanelMessageWidget(provider, QObject::tr("Welcome"), QObject::tr("Hey! You've entered version one of a complete media player based on Material Design. There is still a lot left to accomplish with this project but we'd like to know what you think of the app and Material Design so far. Try listening to some music here while completing <a href=\"https://docs.google.com/forms/d/e/1FAIpQLSe9-OsoFIw2tDGu60GunXBKnDf4WJ1YFkk4pFErPEQnFZGMWw/viewform\">this survey</a>!")) {}

		void onCardDismissed() {
			QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
			if(paths.size() > 0) {
				QDir configDir(paths[0]);
				configDir.mkpath(".");
				QFile file(configDir.filePath("widget_welcome"));
				file.open(QIODevice::WriteOnly);
				file.close();
			}
		}
};

class KnownIssuesWidget : public HomePanelMessageWidget {
	public:
		KnownIssuesWidget(HomeUiProvider & provider) : HomePanelMessageWidget(provider, QObject::tr("Known issues")) {
			QString message = "";
#ifdef Q_OS_LINUX
#ifndef Q_OS_ANDROID
			message = QObject::tr("This app uses GStreamer to load your music and videos, make sure you have all the codecs installed. GStreamer seems to have a few problems where the playback can randomly stops.", "Shown in a card in the home screen on Linux systems.") + " ";
#endif
#endif
#ifdef Q_OS_WIN
			message = QObject::tr("This app uses Windows' multimedia system which means if Windows apps such as Groove can't open a certain file it means your system is missing some codecs.", "Shown in a card in the home screen on Windows systems.") + " " + QObject::tr("Video thumbnails may not show up on some systems.", "Shown in a card in the home screen on Windows systems.") + " ";
#endif
			message += QObject::tr("Sometimes the app can crash when loading many pictures at the same time.", "Shown in a card in the home screen on any system.");
			setMessage(message);
		}

		void onCardDismissed() {
			QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
			if(paths.size() > 0) {
				QDir configDir(paths[0]);
				configDir.mkpath(".");
				QFile file(configDir.filePath("widget_knownissues"));
				file.open(QIODevice::WriteOnly);
				file.close();
			}
		}
};

HomeUiProvider::HomeUiProvider(MediaPlayerContext & context, QQmlApplicationEngine & engine, QObject * parent) : QObject(parent), context(context), engine(engine) {
	bool welcomeDismissed = false, knownIssuesDismissed = false;
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
	if(paths.size() > 0) {
		QDir configDir(paths[0]);
		if(QFile(configDir.filePath("widget_welcome")).exists()) welcomeDismissed = true;
		if(QFile(configDir.filePath("widget_knownissues")).exists()) knownIssuesDismissed = true;
	}
	QString w = "tp";
	if(!welcomeDismissed) w = "w"+w;
	if(!knownIssuesDismissed) w += "i";
	const int len = w.length();
	_widgetCount = len;
	widgets = new char[len];
	for(int i = 0; i < len; i++) widgets[i] = w.at(i).toLatin1();
}

QQuickItem * HomeUiProvider::widgetQmlItemAt(int index) {
	switch(widgets[index]) {
		case 't': {
			QQmlComponent component(&engine, QUrl("qrc:/components/WidgetBaseHorizontalItems.qml"));
			QObject * object = component.create();
			QQuickItem * item = qobject_cast<QQuickItem*>(object);
			new LatestEpisodesWidget(&context, item);
			return item;
		}
		case 'p': {
			QQmlComponent component(&engine, QUrl("qrc:/components/WidgetBaseGridItems.qml"));
			QObject * object = component.create();
			QQuickItem * item = qobject_cast<QQuickItem*>(object);
			new AutomaticPlaylistsWidget(*this, &context, item);
			return item;
		}
		case 'w': return (new WelcomeMessageWidget(*this))->widgetItem();
		case 'i': return (new KnownIssuesWidget(*this))->widgetItem();
		default: return NULL;
	}
}

int HomeUiProvider::widgetCount() {
	return _widgetCount;
}

HomePanelItem::HomePanelItem(const QString & name, const QString & description) : _name(name), _description(description) { }

bool HomePanelItem::enabled() { return _enabled; }
void HomePanelItem::enable() { setEnabled(true); }
void HomePanelItem::disable() { setEnabled(false); }
void HomePanelItem::setEnabled(bool enabled) { _enabled = enabled; }

QString HomePanelItem::name() { return _name; }
QString HomePanelItem::description() { return _description; }

HomePanelUiProvider::HomePanelUiProvider(MediaPlayerContext & context, QObject * parent) : QObject(parent), context(context) { }

Library * HomePanelUiProvider::library() {
	return context.library();
}
