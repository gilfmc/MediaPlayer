#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <cmath>

#include <QtCore/QList>
#include <QtMultimedia/QMediaPlaylist>
#include <QtCore/QAbstractListModel>
#include <QtCore/QRegularExpression>
#include <QtCore/QWaitCondition>

class Playlist;
class PlaylistExtraInfoProvider;

#include "MediaContent.h"
#include "MediaPlayerContext.h"
#include "TemporaryMediaContent.h"
#include "business/MediaMetadataLoader.h"

class ConcurrentMediaInformationLoader : public QObject, public QRunnable/*, MediaReadyNotifyReceiver*/ {
	Q_OBJECT

	public:
		explicit ConcurrentMediaInformationLoader(MediaPlayerContext * context, QList<MediaContent*> & mediaContents, int initialIndex, QObject* parent = nullptr);
		
		virtual void run();
		
		virtual ~ConcurrentMediaInformationLoader();
	
	private slots:
		//void loadMediaData(QMediaPlayer::MediaStatus status);
	
	protected:
		//static const RegexUtils regex;
		
		//QList<MediaContent*> mediaContents;
		//QList<QMediaContent> qMediaContents;
		MediaPlayerContext * context;
		QList<MediaContent*> & _mediaContents;
		const int initialIndex;
		QList<QUrl> urls;
		//MediaContent * currentMedia;
		//bool processMedia;
		
		//QMutex mutex;
		//QWaitCondition mediaPlayerWaiter;
		
		//QMediaPlayer* player;
		
		MediaMetadataLoader * loader;
		
		//virtual Artist* registerArtist(ArtistsLibrary * library, QString name) = 0;
		//void processFeaturing(ArtistsLibrary* artists, QList<Artist*>& list, QString featuring);
		
		virtual void onDoneLoading(int index, MediaContent *) = 0;
};

class SavedPlaylist : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(QString imageUrl READ imageUrl WRITE setImageUrl NOTIFY imageUrlChanged)
	Q_PROPERTY(bool toDelete READ toDelete WRITE setToDelete NOTIFY toDeleteChanged)

	public:
		SavedPlaylist() { }
		SavedPlaylist(const SavedPlaylist & from) {
			_url = from._url;
			_name = from._name;
			_description = from._description;
			_imageUrl = from._imageUrl;
			_extra = from._extra;
			_urls = from._urls;
		}
		SavedPlaylist(const QString & playlistUrl);

		QString name() { return _name; }
		void setName(const QString & name) { _name = name; }
		QString description() { return _description; }
		void setDescription(const QString & description) { _description = description; }
		QString imageUrl() { return _imageUrl; }
		void setImageUrl(const QString & imageUrl) { _imageUrl = imageUrl; }

		bool toDelete() { return _toDelete; }
		void setToDelete(bool toDelete) { _toDelete = toDelete; emit toDeleteChanged(); }

		Q_INVOKABLE
		void deletePlaylist();

		virtual ~SavedPlaylist() { qDebug() << "Deleting this SavedPlaylist (" << _name << ")..."; }

	signals:
		void nameChanged();
		void descriptionChanged();
		void imageUrlChanged();

		void toDeleteChanged();

	private:
		QString _url, _name, _description, _imageUrl, _extra;
		QStringList _urls;
		bool _toDelete = false;

	friend class Playlist;
};
Q_DECLARE_METATYPE(SavedPlaylist)

class Playlist : public QAbstractListModel
{
		Q_OBJECT

		Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
		Q_PROPERTY(int indexToPlayAfter READ indexToPlayAfter WRITE setIndexToPlayAfter NOTIFY indexToPlayAfterChanged)
		Q_PROPERTY(bool isRandom READ isRandom NOTIFY isRandomChanged)

		Q_PROPERTY(QString mediaTitle READ currentMediaTitle NOTIFY currentMediaTitleChanged)
		Q_PROPERTY(QString title READ title NOTIFY titleChanged)
		Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
		Q_PROPERTY(QString imageUrl READ imageUrl NOTIFY imageUrlChanged)
		Q_PROPERTY(bool hasSourceUrl READ hasSourceUrl NOTIFY hasSourceUrlChanged)
		
	public:
		Playlist(MediaPlayerContext * context, QMediaPlaylist * playlist);
		
		void addMedia(QUrl url);
		void addMedia(QList<QUrl> urls);
		Q_INVOKABLE
		void addMedia(int mediaContentId);
		void addMedia(MediaContent * media);
		void addMedia(int at, MediaContent * media);
		void addMedia(QList<MediaContent*> medias);
		Q_INVOKABLE
		void clear();
		Q_INVOKABLE
		void removeMedia(int index);
		
		int currentIndex();

		int indexToPlayAfter();
		void setIndexToPlayAfter(int index);
		
		QUrl urlAt(int index) const;
		QUrl currentUrl() const;
		
		MediaContent * currentMedia() const;
		MediaContent * mediaAt(int index) const;

		QString currentMediaTitle() const;
		
		MediaPlayerContext * context();
		
		Q_INVOKABLE
		void play(int index);

		int loopMode() const;
		void setLoopMode(int mode);

		bool isRandom() const;

		QString title();
		QString defaultDescription();
		QString description();
		QString imageUrl();

		int mediaCount() const;
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QHash< int, QByteArray > roleNames() const;
		
		void onDoneLoading(int i, MediaContent * sender);

		void setExtraInfoProvider(PlaylistExtraInfoProvider * provider);

		Q_INVOKABLE
		bool openFromFile(const QString & url, bool append = false);
		Q_INVOKABLE
		bool open(const QString & title, bool append = false);
		Q_INVOKABLE
		bool open(SavedPlaylist * playlist, bool append = false);
		Q_INVOKABLE
		bool save(const QString & title);
		Q_INVOKABLE
		bool saveAs(const QString & title);
		Q_INVOKABLE
		bool restoreState();
		Q_INVOKABLE
		bool saveState();
		Q_INVOKABLE
		bool saveToFile(const QString & url);
		Q_INVOKABLE
		QVariantList getSavedPlaylists();

		Q_INVOKABLE
		void restoreTemporaryPlaylist();
		Q_INVOKABLE
		void saveTemporaryPlaylist();

		bool hasSourceUrl();

		virtual ~Playlist();
		
	private:
		MediaPlayerContext * _context;
		
		QMediaPlaylist * playlist;
		QList<MediaContent*> mediaContents;
		QList<int> oldMediaContents;
		struct MediaError { QMediaPlayer::Error error; QString message; };
		QMap<int, struct MediaError> errors;
		int * originalIndicesBeforeRandomization = NULL;

		int lastValidCurrentIndex = -1;

		int _indexToPlayAfter = -1;

		PlaylistExtraInfoProvider * extraInfo = NULL;

		class PlaylistConcurrentMediaInformationLoader : public ConcurrentMediaInformationLoader {
			public:
				PlaylistConcurrentMediaInformationLoader(Playlist * playlist, MediaPlayerContext * context, QList<QUrl> urls, int from); //, QObject* parent = nullptr);
				PlaylistConcurrentMediaInformationLoader(Playlist * playlist, MediaPlayerContext * context, QUrl url, int from); //, QObject* parent = nullptr);
				
			protected:
				virtual void onDoneLoading(int index, MediaContent*);

			private:
				Playlist * _playlist;
		};
		
		QString playlistUrl;
		QString playlistTitle;

		inline void notifyPlaylistHeaderChanged();

	public slots:
		void onFilesDropped(QList<QUrl> urls, bool inPlaylist);

		void toggleRandom();
		void randomize();
		void restoreRandomization();

	private slots:
		void durationChanged(qint64);
		//void mediaStatusChanged(QMediaPlayer::MediaStatus);
		//void onCurrentIndexChanged(int);

	signals:
		void currentIndexChanged(int);
		void isRandomChanged(bool);

		void indexToPlayAfterChanged(int);

		void currentMediaTitleChanged();
		
		void currentMediaDataChanged(MediaContent * media);
// 		void onMediaAdded(int from, int to);
// 		void onMediaRemoved(int from, int to);

		void titleChanged();
		void descriptionChanged();
		void imageUrlChanged();
		void hasSourceUrlChanged();
};

class PlaylistExtraInfoProvider : public QObject {
		Q_OBJECT

		friend class Playlist;

	public:
		PlaylistExtraInfoProvider(MediaProperty * property) : property(property) { }

		virtual bool isTitleValid() { return valid; }
		virtual bool isDescriptionValid() { return valid; }
		virtual bool isImageValid() { return valid; }

		virtual QString title() { return property->name(); }
		virtual QString description() {
			if(valid) {
				const int len = playlist->mediaCount();
				if(len == 0) return QString();

				int unknownDurations = 0;
				qint64 duration = 0;
				for(int i = 0; i < len; i++) {
					MediaContent * media = playlist->mediaAt(i);
					if(media) {
						qint64 d = media->length();
						if(d >= 0) {
							duration += d;
						} else {
							unknownDurations++;
						}
					} else {
						unknownDurations++;
					}
				}

				QString result = countMedias(len);
				if(unknownDurations == len) return result;

				result += " \342\200\242 ";
				if(unknownDurations > 0) result += ">";

				if(duration < 5400000) {
					result += tr("%n minute(s)", "", round(duration/60000.0+.2));
				} else {
					result += tr("%n hour(s)", "", round(duration/3600000.0+.2));
				}

				return result;
			}
		}
		virtual QString imageUrl() { return "image://mp/" + property->type() + "/" + QString::number(property->id()); }

		virtual QString countMedias(int count) = 0;

		virtual bool isValid(MediaContent * media) = 0;

		virtual void onPlaylistChanged() {
			const int len = playlist->mediaCount();

			if(len == 0) {
				valid = false;
				return;
			}

			for(int i = 0; i < len; i++) {
				MediaContent * media = playlist->mediaAt(i);
				if(media && media->type() != "...") {
					if(!isValid(media)) {
						valid = false;
						return;
					}
				}
			}

			valid = true;
		}

		MediaInformationManager * mediaInformationManager();

		const QString & mediaPropertyType() { return property->type(); }
		int mediaPropertyId() { return property->id(); }

		static PlaylistExtraInfoProvider * makeProvider(MediaProperty * property);
		static PlaylistExtraInfoProvider * makeProvider(MediaPlayerContext * context, const QString & type, int id);

	protected:
		MediaProperty * property;

	private:
		Playlist * playlist;
		bool valid = true;
};

class ArtistPlaylistInfo : public PlaylistExtraInfoProvider {
	public:
		ArtistPlaylistInfo(MediaProperty * artist) : PlaylistExtraInfoProvider(artist) { }

		virtual QString countMedias(int count) { return PlaylistExtraInfoProvider::tr("%n song(s)", "", count); }

		virtual bool isValid(MediaContent * media) {
			return media->type() == "song" && ((Song*) media)->artist() == property;
		}
};
class TvShowPlaylistInfo : public PlaylistExtraInfoProvider {
	public:
		TvShowPlaylistInfo(MediaProperty * tvShow) : PlaylistExtraInfoProvider(tvShow) { }

		virtual QString countMedias(int count) { return PlaylistExtraInfoProvider::tr("%n episode(s)", "", count); }

		virtual bool isValid(MediaContent * media) {
			if(media->type() == "episode") {
				Episode * e = (Episode*) media;
				if(e->season() && e->season()->parent() == property)
					return true;
			}
			return false;
		}
};

#endif // PLAYLIST_H
