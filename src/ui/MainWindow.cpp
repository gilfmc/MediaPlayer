#include "MainWindow.h"

#include <QtCore/QDebug>
#include <QFileDialog>
#include <QApplication>
#include <QtQuick/QQuickWindow>
#include <QtQml/QQmlContext>

#include <QtCore/QTime>

#include "repo/ArtistsLibrary.h"
#include "business/LibraryScanner.h"

#include "ui/HomeUiProvider.h"

MainWindow::MainWindow(QQmlApplicationEngine &engine, MediaPlayerContext &context) : engine(engine), context(context) {
	//LibraryModel * libraryModel = new LibraryModel(context, this);
	engine.rootContext()->setContextProperty("libraryModel", new LibraryModel(context, this));

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

	QQuickWindow * root = (QQuickWindow*) engine.rootObjects() [0];

	//uiVolume = root->property("volume");
	uiRoot = root;

	root->setProperty("cpp", QVariant::fromValue(this));
	root->setProperty("homeUiProvider", QVariant::fromValue(new HomeUiProvider(context, engine, this)));
	
	lTime = root->findChild<QQuickItem*>("lTime");
	lLength = root->findChild<QQuickItem*>("lLength");
	bMediaProgress = root->findChild<QQuickItem*>("mediaProgress");
	lSubtitle = root->findChild<QQuickItem*>("lSubtitle");
	
	connect(&context, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
	connect(&context, SIGNAL(onPlaybackStateChanged(QMediaPlayer::State)), this, SLOT(onPlaybackStateChanged(QMediaPlayer::State)));
	connect(&context, SIGNAL(playbackRateChanged(qreal)), this, SLOT(playbackRateChanged(qreal)));
	connect(&context, SIGNAL(onSeeked(qint64)), this, SLOT(onSeeked(qint64)));

	connect(&context, SIGNAL(onHasSubtitlesChanged(bool,Subtitles*)), this, SLOT(onHasSubtitlesChanged(bool,Subtitles*)));
	
// 	connect(context.playlist(), SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

	connect(root, SIGNAL(onMediaOpened(QVariant)), this, SLOT(onMediaOpened(QVariant)));
	connect(root, SIGNAL(onVolumeChanged(int)), this, SLOT(onVolumeChanged(int)));

	connect(root, SIGNAL(setCursorVisible(bool)), this, SLOT(setCursorVisible(bool)));
	
	//connect(root, SIGNAL(potatoTest(QString)), this, SLOT(potatoTest(QString)));

	subtitleTimer.setSingleShot(true);
	connect(&subtitleTimer, SIGNAL(timeout()), this, SLOT(subtitleTimerTimeout()));
	
	errorRecoveryTimer.setSingleShot(true);
	errorRecoveryTimer.setInterval(100);
	
	//connect(context.player(), SIGNAL(error(QMediaPlayer::Error)), this, SLOT(onError(QMediaPlayer::Error)));
	connect(context.player(), SIGNAL(playbackRateChanged(qreal)), this, SLOT(subtitleTimerTimeout()));
	connect(&errorRecoveryTimer, SIGNAL(timeout()), this, SLOT(errorRecoveryTimerTimeout()));
//	connect(root, SIGNAL(onPreviousPressed()), this, SLOT(onPreviousPressed()));
//	connect(root, SIGNAL(onNextPressed()), this, SLOT(onNextPressed()));
	
	//uiRoot->findChild<QQuickItem*>("libraryList")->setProperty("model", QVariant(LibraryModel(context, this)));

	QTimer::singleShot(10000, this, [this]() {
		if(!(this->context.isLibraryBeingRefreshed()))
			(new LibraryScanner(&this->context))->start();
	});
}

// void MainWindow::currentIndexChanged(int) {
// 	MediaContent * media = context.playlist()->currentMedia();
// 	if(media->length() <= 0) {
// 		const qint64 duration = context.player()->duration();
// 	}
// }

//void MainWindow::potatoTest(QString test) {
//	if(test == "ce") {
//		ElementToCheck * e = new ElementToCheck("tvShow", "castle");
//		if(context.mediaInformationManager()->checkExistence(e)) {
//			qDebug() << e->realName << "exists (" << e->sourceName + ")";
//		} else {
//			qDebug() << e->sourceName << "doesn't exist";
//		}
//	}
//	Artist * a = context.library()->artists()->lookForArtist(test, 3);
//	if(a) {
//		qDebug() << "I found" << a->name();
//	} else {
//		qDebug() << "I wasn't able to find anything :(";
//	}
//}

void MainWindow::onError(QMediaPlayer::Error error) {
	if (error == QMediaPlayer::ResourceError) {
// 		errorThrownAt = position();
// 		errorThrownState = state();
		//errorRecoveryTimer.start();
		qDebug() << "Playback failed" << context.player()->errorString() << ", trying to recover...";
	} else {
		qDebug() << "Playback failed (" << error << "): " << context.player()->errorString();
	}
}

void MainWindow::errorRecoveryTimerTimeout() {
	qDebug() << "Recovering playback..."; //:\tposition = " << errorThrownAt << "state = " << errorThrownState;
	context.player()->setPosition(bMediaProgress->property("value").toLongLong() + 100);
	context.player()->play();
// 	switch (errorThrownState) {
// 		case QMediaPlayer::PlayingState:
// 			play();
// 			break;
// 		case QMediaPlayer::PausedState:
// 			pause();
// 			break;
// 			//case QMediaPlayer::StoppedState: stop(); break;
// 	}
}

void MainWindow::setCursorVisible(bool visible) {
	QGuiApplication::restoreOverrideCursor();
	//QGuiApplication::overrideCursor()->bitmap()
	if(!visible) QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void MainWindow::durationChanged(qint64 duration) {
	QTime time = QTime::fromMSecsSinceStartOfDay (duration);
	hasHour = time.hour() > 0;
	if(hasHour) lLength->setProperty("text", time.toString("H:mm:ss"));
	else lLength->setProperty("text", time.toString("mm:ss"));
	mediaTick();
}

void MainWindow::onSeeked(qint64 position) {
	QTime time = QTime::fromMSecsSinceStartOfDay (position);
	if(hasHour) lTime->setProperty("text", time.toString("H:mm:ss"));
	else lTime->setProperty("text", time.toString("mm:ss"));
	if(context.hasSubstitles()) {
		context.subtitles()->moveToTime(position);
		subtitleTimerTimeout();
	}
}

bool MainWindow::subtitlesEnabled() {
	return _subtitlesEnabled;
}

void MainWindow::setSubtitlesEnabled(bool enabled) {
	if(_subtitlesEnabled == enabled) return;
	_subtitlesEnabled = enabled;
	if(enabled) {
		Subtitles * subtitles = context.subtitles();
		if(subtitles == NULL) return;
		subtitles->moveToTime(context.mediaPosition());
		subtitleTimerTimeout();
	} else {
		subtitleTimer.stop();
		lSubtitle->setProperty("text", "");
	}
}

int MainWindow::subtitleDelay() {
	return _subtitleDelay;
}

void MainWindow::setSubtitleDelay(int delay) {
	_subtitleDelay = delay;
	subtitleTimerTimeout();
}

void MainWindow::loadSubtitlesFromFile(QUrl file) {
	context.loadSubtitles(file);
}

void MainWindow::onHasSubtitlesChanged(bool has, Subtitles * subtitles) {
	if(has) {
		subtitles->moveToTime(context.mediaPosition());
		subtitleTimerTimeout();
	} else
		lSubtitle->setProperty("text", "");
}

void MainWindow::subtitleTimerTimeout() {
	Subtitles * subtitles = context.subtitles();
	if(subtitles == NULL) return;

	qint64 pos = context.mediaPosition() + _subtitleDelay;
	Subtitle * subtitle = subtitles->getAtCursor();
	if(pos >= subtitle->start) {
		if(pos >= subtitle->end) {
			subtitles->moveToNext();
			lSubtitle->setProperty("text", "");
			if(subtitles->hasNext()) {
				subtitleTimerTimeout();
//				subtitleTimer.setInterval(subtitles->getAtCursor()->start - context.mediaPosition());
//				subtitleTimer.start();
			}
		} else {
			lSubtitle->setProperty("text", subtitle->text);
			if(context.playbackState() == QMediaPlayer::PlayingState) {
				subtitleTimer.setInterval((subtitle->end - pos) / context.playbackRate());
				subtitleTimer.start();
			}
		}
	} else {
		if(context.playbackState() == QMediaPlayer::PlayingState) {
			subtitleTimer.setInterval((subtitle->start - pos) / context.playbackRate());
			subtitleTimer.start();
		}
	}
}

MediaContentUiProvider * MainWindow::getMediaContentUiProvider(const QString & type, int id) {
	return new MediaContentUiProvider(&context, context.library()->getMediaProperty(type, id), this);
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::State state) {
	if(state == QMediaPlayer::PlayingState) {
		if(timer == NULL) {
			timer = new QTimer();
			qreal rate = context.playbackRate();
			if(rate > 1) timer->setTimerType(Qt::CoarseTimer);
			else timer->setTimerType(Qt::VeryCoarseTimer);
			timer->setInterval(std::abs(1000 * rate));
			
			connect(timer, SIGNAL(timeout()), this, SLOT(mediaTick()));
		}
		mediaTick();
		timer->start();
		if(context.hasSubstitles())
			subtitleTimerTimeout();
	} else if(timer) {
		timer->stop();
		subtitleTimer.stop();
	}
}

void MainWindow::mediaTick() {
	QTime time = QTime::fromMSecsSinceStartOfDay (context.mediaPosition());
	if(hasHour) lTime->setProperty("text", time.toString("H:mm:ss"));
	else lTime->setProperty("text", time.toString("mm:ss"));
}

void MainWindow::playbackRateChanged(qreal rate) {
	if(timer) {
		//qDebug() << "time updater timer now updates every
		if(rate > 1) timer->setTimerType(Qt::CoarseTimer);
		else timer->setTimerType(Qt::VeryCoarseTimer);
		timer->setInterval(std::abs(1000 / rate));
	}
	if(context.hasSubstitles())
		subtitleTimerTimeout();
}

//void MainWindow::onPlayClicked() {
//	QQuickWindow * window = (QQuickWindow*) engine.rootObjects()[0];
//	QFileDialog::getOpenFileName(0, "Test thing");
//}

void MainWindow::onMediaOpened(QVariant urls) {
	QMediaPlayer * player = this->player();
	Playlist * playlist = context.playlist();
	//playlist->clear();
	playlist->addMedia(urls.value<QList<QUrl>>());
// 	foreach (QUrl url, urls.value<QList<QUrl>>()) {
// 		playlist->addMedia(url);
// 	}

	if(player->state() != QMediaPlayer::PlayingState)
		player->play();
}

//void MainWindow::onPreviousPressed() {
//	context.setPlaybackRate(player()->playbackRate() - 0.25);
//}

//void MainWindow::onNextPressed() {
//	context.setPlaybackRate(player()->playbackRate() + 0.25);
//}

QMediaPlayer * MainWindow::player() {
	return context.player();
}

void MainWindow::onVolumeChanged(int volume) {
	context.setVolume(volume);
}

void MainWindow::raise() {
	uiRoot->requestActivate();
	uiRoot->raise();
}

bool MainWindow::isFullscreen() {
	return uiRoot->windowState() == Qt::WindowFullScreen;
}
//#include <QtGui/QDesktopWidget>
void MainWindow::setFullscreen(bool fullscreen) {
	if (isFullscreen() && !fullscreen) uiRoot->setWindowState(Qt::WindowNoState);
	else if (!isFullscreen() && fullscreen) {
		uiRoot->setWindowState(Qt::WindowFullScreen);
		//toW = QApplication::desktop()->width();
		//toH = QApplication::desktop()->height();
		//timerEvent(0);
	}
}

// void MainWindow::timerEvent(QTimerEvent* e) {
// 	//QObject::timerEvent(e);
// 	bool widthDone = false;
// 	if(uiRoot->width() < toW) {
// 		uiRoot->setWidth(uiRoot->width() + 1);
// 	} else widthDone = true;
// 	bool heightDone = false;
// 	if(uiRoot->height() < toH) {
// 		uiRoot->setHeight(uiRoot->height() + 1);
// 	} else heightDone = true;
// 	
// 	if(widthDone == false || heightDone == false)
// 		startTimer(20, Qt::PreciseTimer);
// }

void MainWindow::quit() {
	context.stop();
	QApplication::quit();
}

MainWindow::~MainWindow() {
	if(timer) delete timer;
}

LibraryModel::LibraryModel(MediaPlayerContext& context, QObject * parent): QAbstractListModel(parent), context(&context), library(context.library()) {
	//QQmlApplicationEngine::setObjectOwnership(this, QQmlApplicationEngine::CppOwnership);

	connect(context.library(), SIGNAL(onMediaPropertyAddedToLibrary(const QString &, MediaProperty *)), this, SLOT(onMediaPropertyAddedToLibrary(const QString &, MediaProperty *)));
	connect(context.library(), SIGNAL(onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *)), this, SLOT(onMediaPropertyRemovedFromLibrary(const QString &, MediaProperty *)));
	connect(context.library(), &Library::onMediaPropertyRenamed, [=](const QString & type, MediaProperty * item, const QString &) {
		const int len = items.length();
		for(int i = 0; i < len; i++) {
			if(items.at(i)->media == item) {
				items[i] = new LibraryListItem(item);
				QModelIndex mi = index(i);
				QVector<int> roles;
				roles << Qt::UserRole + 4;
				emit dataChanged(mi, mi, roles);
				break;
			}
		}
		// TODO reorder the list
	});

	items.append(new LibraryListItem(tr("Music"), "av/library_music"));
	foreach(Artist * a, library->artists()->naturallySortedList()) {
		items.append(new LibraryListItem(a));
	}
	items.append(new LibraryListItem(tr("TV Shows"), "hardware/tv"));
	foreach(TvShow * ts, library->tvShows()->naturallySortedList()) {
		items.append(new LibraryListItem(ts));
	}

	items.append(new LibraryListItem(tr("Movies"), "av/movie"));
}

void LibraryModel::onMediaPropertyAddedToLibrary(const QString & type, MediaProperty * item) {
	//qDebug() << type << "registered (" << item << ")";
	if(type == "artist") {
		int i = 0;
		Artist * a;
		foreach(a, library->artists()->naturallySortedList()) {
			i++;
			if(a == item) break;
		}
		beginInsertRows(QModelIndex(), i, i);
		items.insert(i, new LibraryListItem(a));
		endInsertRows();
	} else if(type == "tvShow") {
		int i = 0;
		TvShow * ts;
		foreach(ts, library->tvShows()->naturallySortedList()) {
			if(ts == item) break;
			i++;
		}
		i = library->artists()->count() + 2;
		beginInsertRows(QModelIndex(), i, i);
		items.insert(i, new LibraryListItem(ts));
		endInsertRows();
	}
}

void LibraryModel::onMediaPropertyRemovedFromLibrary(const QString & type, MediaProperty * item) {
	const int len = items.length();
	for(int i = 0; i < len; i++) {
		if(items.at(i)->media == item) {
			beginRemoveRows(QModelIndex(), i, i);
			items.removeAt(i);
			endRemoveRows();
			return;
		}
	}
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const {
	int i = index.row();
	
	role -= Qt::UserRole;
	// qDebug() << "data(" << i << "," << role << ")";
	switch(role) {
		case 0: return i;
		case 1: return items[i]->media ? items[i]->media->id() : -1;
		case 2: return items[i]->media ? items[i]->media->type() : "";
		case 3: return items[i]->image;
		case 4: return items[i]->name;
		case 5: return items[i]->type;
	}
	
	return QVariant();
}

int LibraryModel::rowCount(const QModelIndex&) const {
	return items.count();
}

QHash< int, QByteArray > LibraryModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "i";
	roles[Qt::UserRole + 1] = "id";
	roles[Qt::UserRole + 2] = "mediaType";
	roles[Qt::UserRole + 3] = "image";
	roles[Qt::UserRole + 4] = "title";
	roles[Qt::UserRole + 5] = "type";
	return roles;
}

MediaContentUiProvider * LibraryModel::getItem(int index) const {
	MediaProperty * m = items[index]->media;
	return m ? new MediaContentUiProvider(context, m) : NULL;
}

void LibraryModel::playAll(int index) {
	playAll(items[index]->media);
}

void LibraryModel::playAll(const QString & type, int id) {
	playAll(library->getMediaProperty(type, id));
}

void LibraryModel::playAll(MediaProperty * m) {
	if(m) {
		Playlist * playlist = context->playlist();
		if(playlist->mediaCount() == 0) {
			playlist->setExtraInfoProvider(PlaylistExtraInfoProvider::makeProvider(m));
		}
		playlist->addMedia(((MediaContentContainer*)m)->getMediaContents());
	}
}

void LibraryModel::removeFromLibrary(int index) {
	removeFromLibrary(items[index]->media);

}

void LibraryModel::removeFromLibrary(const QString & type, int id) {
	removeFromLibrary(library->getMediaProperty(type, id));
}

void LibraryModel::removeFromLibrary(MediaProperty * m) {
	if(m) {
		lastRemovedItem = m;
		if(m->type() == "artist") library->artists()->removeFromLibrary((Artist*)m);
		else if(m->type() == "tvShow") library->tvShows()->removeFromLibrary((TvShow*)m);
	}
}

void LibraryModel::addToLibrary(const QString & type, int id) {
	addToLibrary(library->getMediaProperty(type, id));
}

void LibraryModel::addToLibrary(MediaProperty * m) {
	if(m) {
		if(m->type() == "artist") library->artists()->addToLibrary((Artist*)m);
		else if(m->type() == "tvShow") library->tvShows()->addToLibrary((TvShow*)m);
	}
}

void LibraryModel::renameMediaProperty(const QString & type, int id, const QString & newName) {
	MediaProperty * m = library->getMediaProperty(type, id);
	if(m) {
		if(m->type() == "artist") library->artists()->rename((Artist*)m, newName);
		else if(m->type() == "tvShow") library->tvShows()->rename((TvShow*)m, newName);
	}
}

void LibraryModel::undoRemoval() {
	addToLibrary(lastRemovedItem);
	lastRemovedItem = NULL;
}

QString LibraryModel::getMediaPropertyName(const QString & type, int id) {
	MediaProperty * m = library->getMediaProperty(type, id);
	if(m) return m->name();
	return QString();
}

QString LibraryModel::getSingularItemName(int index) {
	switch (getGroupIndex(index)) {
		case 0: return tr("artist");
		case 1: return tr("TV show");
		case 2: return tr("movie");
		default: return QString();
	}
}

QString LibraryModel::getInternalGroupName(int index) {
	switch (getGroupIndex(index)) {
		case 0: return "artist";
		case 1: return "tvShow";
		case 2: return "movie";
		default: return QString();
	}
}

MediaContentUiProvider * LibraryModel::registerItem(QString group, QString name) {
	if(group == "artist")
		return new MediaContentUiProvider(context, context->library()->artists()->registerArtist(new TemporaryArtist(name), NULL));
	else if(group == "tvShow")
		return new MediaContentUiProvider(context, context->library()->tvShows()->registerTvShow(new TemporaryTvShow(name), NULL));

	return NULL;
}

PropertyListUiProvider * LibraryModel::getListUiAt(int groupAt) {
	return getListUi(getGroupIndex(groupAt));
}

PropertyListUiProvider * LibraryModel::getListUi(int index) {
	switch(index) {
		case 0: return new MusicListUiProvider(*context);
		case 1: return new TvShowsListUiProvider(*context);
		default: return NULL;
	}
}

int LibraryModel::getGroupIndex(int at) {
	int gi = 0;
	for(int i = 0; i < at; i++) {
		if(items[i]->type == 0) gi++;
	}
	return gi;
}
