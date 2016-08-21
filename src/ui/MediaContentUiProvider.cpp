#include "MediaContentUiProvider.h"

class InfoLoader : public QRunnable {
	public:
		InfoLoader(MediaContentUiProviderInterface * i, MediaPlayerContext * context, MediaProperty * media, bool & loadedInfo, QMap<QString, QVariant> & info, MediaInformationRequestOptions options) :
			i(i),
			context(context),
			media(media),
			loadedInfo(&loadedInfo),
			info(&info),
			options(options){}

		virtual void run() {
			context->mediaInformationManager()->getInformation(media, * info, options);
			if(!options.cancel)
				i->notifyDoneLoadingInformation();
		}
		
		void notifyParentDeleted() {
			options.cancel = true;
		}

	private:
		MediaContentUiProviderInterface * i;
		MediaPlayerContext * context;
		MediaProperty * media;
		bool * loadedInfo;
		QMap<QString, QVariant> * info;
		MediaInformationRequestOptions options;
};

MediaContentUiProvider::MediaContentUiProvider(MediaPlayerContext * context, MediaProperty * media, QObject * parent) : QObject(parent), context(context), media(media), _model(new MediaContentUiModel(context, media, this)) { }

const QString & MediaContentUiProvider::name() {
	return media->name();
}

const QString MediaContentUiProvider::description() {
	if(loadedInfo) {
		QString field = media->type() == "tvShow" || media->type() == "movie" ? "plot" : "profile";
		if(info.contains(field)) return info[field].toString();
		else return QString();
	}

	load();
	return QString();
}

const QString MediaContentUiProvider::posterUrl() {
	if(loadedInfo) {
		QString key;
		if(info.contains("picture")) key = "picture";
		else if(info.contains("poster")) key = "poster";

		if(!key.isEmpty()) {
			QString url = info[key].toString();
			if(!url.isEmpty()) {
				url = context->mediaInformationManager()->getImageUrl(url);
				if(QFile(url).exists()) {
					return "file://" + url;
				} else if(!refreshingToRemakeCache) {
					refreshingToRemakeCache = true;
					load(true);
				}
			}
		}

		return QString();
	}

	load();
	return QString();
}

void MediaContentUiProvider::load(bool refresh) {
	if(loading) return;
	loading = true;
	MediaInformationRequestOptions options;
	options.refresh = refresh ? MediaInformationRequestOptions::ForceRefresh : MediaInformationRequestOptions::RefreshIfNeeded;
	InfoLoader * loader = new InfoLoader(this, context, media, loadedInfo, info, options);
	this->loader = loader;
	QThreadPool::globalInstance()->start(loader);
}

void MediaContentUiProvider::notifyDoneLoadingInformation() {
	loader = NULL;
	loading = false;
	loadedInfo = true;
	emit descriptionChanged();
	emit posterUrlChanged();
}

int MediaContentUiProvider::mediaId() {
	return media ? media->id() : -1;
}

const QString & MediaContentUiProvider::mediaType() {
	return media ? media->type() : QString::null;
}

MediaContentUiModel * MediaContentUiProvider::model() {
	QQmlEngine::setObjectOwnership(_model, QQmlEngine::CppOwnership);
	return _model;
}

MediaContentUiModelGroupItem * MediaContentUiModel::getGroupItem(int index) {
	QQmlEngine::setObjectOwnership(items[index], QQmlEngine::CppOwnership);
	return items[index];
}

MediaListUiProvider * MediaContentUiModel::makeListUiProvider(int index) {
	const QString type = property->type();
	if(type == "tvShow") {
		if(index == 0) return new EpisodesListUiProvider((TvShow*) property);
		else return new SeasonsListUiProvider((TvShow*) property);
	} else if(type == "artist") {
		if(index == 0) return new SongsListUiProvider((Artist*) property);
	}

	return new MediaListUiProvider();
}

MediaListUiProvider * MediaContentUiModel::getListUiProvider(int index) {
	return makeListUiProvider(index);

//	MediaListUiProvider * ui = makeListUiProvider(index);
//	QQmlEngine::setObjectOwnership(ui, QQmlEngine::CppOwnership);

//	return ui;
}

QVariant MediaContentUiModelGroupItem::data(const QModelIndex& index, int role) const {
	int i = index.row();

	role -= Qt::UserRole;
	switch(role) {
		case 0: return i;
		case 1: return items[i]->getId();
		case 2: return items[i]->getNumber();
		case 3: return items[i]->getTitle();
		case 4: return items[i]->getDescription();
		case 5: return items[i]->getLength();
		case 6: return items[i]->getDate();
	}

	return QVariant();
}

int MediaContentUiModelGroupItem::rowCount(const QModelIndex&) const {
	return items.count();
}

QHash< int, QByteArray > MediaContentUiModelGroupItem::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "i";
	roles[Qt::UserRole + 1] = "id";
	roles[Qt::UserRole + 2] = "number";
	roles[Qt::UserRole + 3] = "title";
	roles[Qt::UserRole + 4] = "description";
	roles[Qt::UserRole + 5] = "length";
	roles[Qt::UserRole + 6] = "date";
	//roles[Qt::UserRole + 6] = "lastColumn";
	return roles;
}

class SeasonUiItem : public MediaContentUiModelItem {
	public:
		SeasonUiItem(MediaPlayerContext * context, Season * season) : MediaContentUiModelItem(context, season) { }

		virtual const QString getTitle() {
			Season * s = (Season*) media;
			if(s->name().isEmpty()) return QObject::tr("Season %1").arg(s->number());
			else return s->name();
		}
		virtual const QString getDescription() {
			return QObject::tr("%n episode(s)", "", ((Season*)media)->episodeCount());
		}
};

class EpisodeUiItem : public MediaContentUiModelItem {
	public:
		EpisodeUiItem(MediaPlayerContext * context, Episode * episode) : MediaContentUiModelItem(context, episode) { }

		virtual const QString getTitle() {
			Episode * e = (Episode*) media;
			if(e->name().isEmpty()) return QObject::tr("Episode %1").arg(e->number());
			else return QString::number(e->number()) + " - " + e->name();
		}
		virtual const QString getDescription() {
			Episode * e = (Episode*) media;
			if(e->season()) return QObject::tr("Season %1").arg(e->season()->number());
			else return "";
		}
		virtual const QString getLength() {
			return context->formatLength((MediaContent*)media);
		}
		virtual const QString getDate() {
			Episode * e = (Episode*) media;
			ImpreciseDate date = e->date();
			return date.hasYear() ? QString::number(date.dayOfMonth()) + "/" + QString::number(date.month()) + "/" + QString::number(date.year()) : "";
		}
};

MediaContentUiModel::MediaContentUiModel(MediaPlayerContext *context, MediaProperty *property, QObject *parent) :
		context(context),
		property(property),
		QAbstractListModel(parent) {
	const QString & type = property->type();
	MediaContentUiModelGroupItem * group;
	MediaContentUiModelItem * item;
	if(type == "artist") {
		int limit = 5;
		items.append((group = new MediaContentUiModelGroupItem(1, tr("Songs"))));
		group->emptyMessage = tr("You don't have any songs by %1 in your library but you can add some.").arg(property->name());
		QList<MediaContent*> medias = ((Artist*)property)->getMediaContents();
		for(MediaContent * m : medias) {
			if(m) {
				group->items.append((item = new MediaContentUiModelItem(context, m)));
				item->media = NULL;
				limit--;
			}
			if(limit == 0) break;
		}
		for(MediaContent * m : medias) delete m;
		QList<MediaContent*> featuredIn = ((Artist*)property)->featuredIn();
		if(featuredIn.length() > 0) {
			group = new MediaContentUiModelGroupItem(1, tr("Participated in"));
			items.append(group);
			limit = 5;
			for(MediaContent * m : featuredIn) {
				if(m) {
					group->items.append((item = new MediaContentUiModelItem(context, m)));
					item->media = NULL;
					limit--;
				}
				if(limit == 0) break;
			}
			for(MediaContent * m : featuredIn) delete m;
		}
		items.append((group = new MediaContentUiModelGroupItem(2, tr("Albums"))));
		group->emptyMessage = tr("Albums are not supported yet.");
		group->emptyIcon = "av/album";
	} else if(type == "tvShow") {
		TvShow * tvShow = (TvShow*)property;
		const QList<Season*>& seasons = tvShow->seasons();
		items.append((group = new MediaContentUiModelGroupItem(2, tr("Episodes"))));
		for(Season * s : seasons) {
			if(s) {
				const QList<Episode*> & episodes = s->episodes();
				if(episodes.length() > 0) {
					group->items.insert(0, new EpisodeUiItem(context, episodes.first()));
					if(episodes.length() > 1) {
						group->items.insert(0, new EpisodeUiItem(context, episodes.last()));
					}
				}
			}
		}
		items.append((group = new MediaContentUiModelGroupItem(1, tr("Seasons"))));
		group->setShowNumber(false);
		for(Season * s : seasons) {
			if(s) group->items.append(new SeasonUiItem(context, s));
		}
		items.append((group = new MediaContentUiModelGroupItem(3, tr("Staring"))));
		group->emptyMessage = tr("Cast not supported yet.");
	}
}

QVariant MediaContentUiModel::data(const QModelIndex& index, int role) const {
	int i = index.row();

	role -= Qt::UserRole;
	switch(role) {
		case 0: return i;
		case 1: return items[i]->type;
		case 2: return items[i]->title;
		case 3: return items[i]->emptyMessage;
		case 4: return items[i]->emptyIcon;
	}

	return QVariant();
}

int MediaContentUiModel::rowCount(const QModelIndex&) const {
	return items.count();
}

QHash< int, QByteArray > MediaContentUiModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole] = "i";
	roles[Qt::UserRole + 1] = "type";
	roles[Qt::UserRole + 2] = "title";
	roles[Qt::UserRole + 3] = "emptyMessage";
	roles[Qt::UserRole + 4] = "emptyIcon";
	return roles;
}

void MediaContentUiModel::playAll(int index) {
//	MediaProperty * m = items[index]->media;
//	if(m) {
//		if(m->type() == "artist") context->playlist()->addMedia(((Artist*)m)->getMediaContents());
//		else if(m->type() == "tvShow") context->playlist()->addMedia(((TvShow*)m)->getMediaContents());
//	}
}

void MediaContentUiModel::addToPlaylist(int groupIndex, int index) {
	MediaProperty * media = items.at(groupIndex)->items.at(index)->media;
	if(media == NULL) media = context->library()->mediaContents()->get(items.at(groupIndex)->items.at(index)->getId());
	if(media) {
		Playlist * playlist = context->playlist();
		if(playlist->mediaCount() == 0) {
			playlist->setExtraInfoProvider(PlaylistExtraInfoProvider::makeProvider(property));
		}
		if(MediaContentContainer * container = dynamic_cast<MediaContentContainer*>(media)) {
			playlist->addMedia(container->getMediaContents());
		} else if(MediaContent * content = dynamic_cast<MediaContent*>(media)) {
			playlist->addMedia(content);
		}
	}
}

void MediaContentUiModel::play(int groupIndex, int index) {
	int count = context->playlist()->mediaCount();
	addToPlaylist(groupIndex, index);
	context->playlist()->play(count);
}

void MediaContentUiModel::onMediaPropertyRegistered(const QString, MediaProperty *) {

}

MediaContentUiProvider::~MediaContentUiProvider() {
	if(loader) {
		((InfoLoader*)loader)->notifyParentDeleted();
	}
	qDebug() << "Deleting MediaContentUiProvider...";
	delete _model;
}
