unix:!android {
	target.path = /usr/bin

	app.path = /usr/share/applications
	app.files += PapyrosMediaPlayer.desktop

	icon.path = /usr/share/icons/hicolor/192x192/apps/
	icon.files += papyrosmediaplayer.png

	appTranslations.path = /usr/share/papyrosmediaplayer/translations
	appTranslations.files += translations/*.qm

	INSTALLS += target icon app appTranslations
}

export(INSTALLS)
