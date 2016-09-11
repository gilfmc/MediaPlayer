#include <QApplication>

#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QPluginLoader>

#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QDir>

#include "MediaPlayerContext.h"
#include "ui/MainWindow.h"
#include "ui/MediaContentUiProvider.h"
#include "ui/MediaListUiProvider.h"
#include "ui/SearchUi.h"

#include "business/InformationDetectorPlugin.h"
#include "business/PluginManager.h"
#include "business/MediaImageProvider.h"

int main(int argc, char *argv[])
{
	//for(int i = 0; i<argc; i++) qDebug() << argv[0];
	QApplication app(argc, argv);
	app.setApplicationName("Papyros Media Player");
	app.setOrganizationName("Gil Castro");
	app.setApplicationVersion("0.1");
	app.setWindowIcon(QIcon(":/icon.png"));

	QTranslator qtTranslator;
	qtTranslator.load("./translations/" + QLocale::system().name(),
					QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	QTranslator translator;
	if(!translator.load("./translations/" + QLocale::system().name())) {
		translator.load("/usr/share/papyrosmediaplayer/translations/" + QLocale::system().name());
	}
	app.installTranslator(&translator);

	qmlRegisterType<MainWindow>();
	qmlRegisterType<MediaPlayerContext>("org.papyros.mediaplayer", 1, 0, "MediaPlayerContext");
	qmlRegisterType<MediaContentUiProvider>("org.papyros.mediaplayer", 1, 0, "MediaContentUiProvider");
	qmlRegisterType<MediaContentUiModelGroupItem>("org.papyros.mediaplayer", 1, 0, "MediaContentUiModelGroupItem");
	qmlRegisterType<MediaListUiProvider>("org.papyros.mediaplayer", 1, 0, "MediaListUiProvider");
	qmlRegisterType<PropertyListUiProvider>("org.papyros.mediaplayer", 1, 0, "PropertyListUiProvider");
	qmlRegisterType<SavedPlaylist>("org.papyros.mediaplayer", 1, 0, "SavedPlaylist");
	qmlRegisterType<SearchUi>("org.papyros.mediaplayer", 1, 0, "SearchUi");
	qmlRegisterType<SearchGroup>("org.papyros.mediaplayer", 1, 0, "SearchGroup");

	MediaPlayerContext context;

	QQmlApplicationEngine engine;
	engine.addImageProvider("mp", new MediaImageProvider(context));
	QQmlEngine::setObjectOwnership(&context, QQmlEngine::CppOwnership);
	engine.rootContext()->setContextProperty("app", &app);
	engine.rootContext()->setContextProperty("context", &context);
	engine.rootContext()->setContextProperty("_context", &context);
	engine.rootContext()->setContextProperty("mediaPlayer", context.player());
	//engine.findChild<QDeclarativeVideoOutput>("videoOutput").;

	//context.player()->setVideoOutput((QVideoWidget*) engine.findChild<QObject*>("videoOutput"));
	//qDebug() << "Got: " << engine.rootContext()->findChild<QObject*>("videoOutput", Qt::FindChildrenRecursively);

	MainWindow mainWindow(engine, context);
	context.startMpris(&mainWindow);
	engine.rootContext()->setContextProperty("ui", &mainWindow);
// 	qDebug() << context.library()->artists()->count() << "artists";
// 	for(MediaPropertyLibrary<Artist*>::Iterator * i = context.library()->artists()->iterate(); i->next();) {
// 		qDebug() << i->item()->name();
// 	}
	// test stuff:
// 	MediaContentInformationContainer * mcic = new MediaContentInformationContainer();
// 	InformationDetectorPlugin * idp = context.pluginManager()->informationDetector(0)->instance(); // qobject_cast<InformationDetectorPlugin*>(plugin.instance());
// 	if(idp) {
// 		idp->detectInformation(mcic);
// 		qDebug() << "Received" << mcic->title;
// 	} else {
// 		qDebug() << "The plugin failed to load :(";
// 	}
	//

	if(argc > 1) {
		Playlist * playlist = context.playlist();
		int initialItem = playlist->mediaCount();
		for(int i = 1; i < argc; i++) {
			QString arg(argv[i]);
			if(arg.startsWith("--")) {
				if(arg == "--restore-state-and-play") {
					if(!context.settings()->restoreState()) playlist->restoreState();
					context.play();
					initialItem = -1;
					break;
				}
			} else {
				if(arg.endsWith(".m3u")) {
					playlist->openFromFile(arg);
					context.play();
					break;
				} else {
					QUrl url(arg);
					playlist->addMedia(url);
				}
			}
		}
		if(initialItem >= 0) playlist->play(initialItem);
	}

	return app.exec();
}
