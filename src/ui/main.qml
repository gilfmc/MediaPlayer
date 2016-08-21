import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0

import QtMultimedia 5.5

import Material 0.2
import Material.ListItems 0.1 as ListItems
import QtQuick.Controls.Styles.Material 0.1 as MaterialStyle

import org.papyros.mediaplayer 1.0

import "components"
import "dialogs"

ApplicationWindow {
    id: window

    visible: true
    width: { return context.settings.mainWindowWidth; }
    height: { return context.settings.mainWindowHeight; }

    minimumWidth: Units.dp(460)
    minimumHeight: barControls.height

    title: "Papyros Media Player"

    theme {
        primaryColor: context.settings.primaryColor // Palette.colors.orange["500"]
        //primaryDarkColor: Palette.colors.orange["700"]
        accentColor: context.settings.accentColor // Palette.colors.orange["A400"]
    }

    Timer {
        property int suggestReplayAt;

        id: mediaTimeUpdater
        repeat: true
        onTriggered: {
            var pos = context.mediaPosition;
            mediaProgress.value = pos;
            bReplay.showReplay = suggestReplayAt > 0 ? pos > suggestReplayAt : false;
        }
    }

    Component.onCompleted: {
//        if(Device.type == Device.desktop) {
//            //Units.pixelDensity /= 1.4;

//            width = context.settings.mainWindowWidth;
//            height = context.settings.mainWindowHeight;
//        }
        Units.multiplier = context.settings.dpMultiplier(Units.multiplier);

        context.durationChanged.connect(function(duration) {
            if(duration > 20000) {
                mediaTimeUpdater.suggestReplayAt = duration - Math.min(Math.max(duration * 0.07, 10000), 150000);
            } else {
                mediaTimeUpdater.suggestReplayAt = -1;
            }

            mediaProgress.maximumValue = duration;
            mediaTimeUpdater.interval = Math.max(duration/mediaProgress.width, 300);
            //console.log("updating the progress bar every", mediaTimeUpdater.interval, "milliseconds");
        });
        context.onSeeked.connect(function(position) {
            if(mediaProgress.maximumValue < position) {
                mediaProgress.setValueWhenPossible = position;
            } else {
                mediaProgress.value = position;
            }
        });
        context.onPlaybackStateChanged.connect(function(state) {
            if(state === MediaPlayer.PlayingState) {
                mediaTimeUpdater.start();
            } else {
                mediaTimeUpdater.stop();
            }
            updateVideoUi(context.videoAvailable);
        });
//        context.isPlayingChanged.connect(function(playing) {
//            if(playing) mediaTimeUpdater.start();
//            else mediaTimeUpdater.stop();
//            //console.log("the progress bar updater is now", mediaTimeUpdater.running ? "running" : "stopped");
//        });
        context.videoAvailableChanged.connect(updateVideoUi);
        context.volumeChanged.connect(function(volume) {
          sVolume.value = volume;
        });

        if(context.settings.restoreState && !context.playlist.restoreState()) console.log("Failed to restore state.");
        //libraryList.model = libraryModel;
        //onDoneLoadingGui();
    }

    onWidthChanged: {
        if(context.hasMediaToPlay) {
            mediaTimeUpdater.interval = Math.max(context.mediaDuration/mediaProgress.width, 300);
            //console.log("window resized... updating the progress bar every", mediaTimeUpdater.interval, "milliseconds");
        }
    }

    onActiveChanged: {
        if(videoBehind && !active)  {
            baseVideo.basicOsdVisible = false;
            barControls.showing = false;
            barControls.peeking = false;
        }
    }

    onClosing: {
        if(/*context.settings.restoreState &&*/ !context.playlist.saveState()) console.log("Failed to save current state before closing.");
        app.quit();
    }

    function updateVideoUi(videoAvailable) {
        var isStopped = context.playbackState === MediaPlayer.StoppedState;
        if(videoBehind && !userOpenedOsd && (!videoAvailable || isStopped)) {
            barControls.showing = true;
            barControls.peeking = false;
            osdOpen = false;
            cursorVisible = true;
        }

        videoBehind = isStopped ? false : videoAvailable;
    }

    property var cpp;

    property bool videoBehind: false;
	property bool osdOpen: false;
    property bool userOpenedOsd: false

    property var mediaOpenerDialog: undefined
    //    signal onPlayClicked
    signal onMediaOpened(variant urls)
    signal onVolumeChanged(int value)
//    signal onPreviousPressed
//    signal onNextPressed
    signal setCursorVisible(bool visible)

    //signal onDoneLoadingGui()

    property alias volume: sVolume.value

    property bool cursorVisible: true
    onCursorVisibleChanged: setCursorVisible(cursorVisible)

    property var homeUiProvider
    onHomeUiProviderChanged: {
        navigator.openPage("HomePage", false, {"ui": homeUiProvider});
    }

    QtObject {
        id: uiState

        readonly property alias videoBehind: window.videoBehind;
        property alias osdOpen: window.osdOpen;
        property alias bottomBarHeight: barControls.height;

        function openAddMediaPropertyDialog(title, group, replacePages) {
            dialogAddPropertyToLibrary.open(title, group, replacePages);
        }
        function openRenameMediaPropertyDialog(type, id) {
            dialogRenameMediaProperty.open(type, id);
        }

        function openMediaPropertyInfoPage(type, id) {
            if(videoBehind && !osdOpen) osdOpen = true;
            var ui = cpp.getMediaContentUiProvider(type, id);
            if(ui) navigator.openPage("MediaContentPage", true).ui = ui;
        }

        function showSnackbar(text, time, button, onClicked) {
            snackbar.show(text, time, button, onClicked);
        }

        function playAll(type, id) {
            libraryModel.playAll(type, id);
        }

        function addToLibrary(type, id) {
            libraryModel.addToLibrary(type, id);
        }
        function removeFromLibrary(type, id) {
            showMediaPropertyRemovalSnackbar(libraryModel.getMediaPropertyName(type, id));
            libraryModel.removeFromLibrary(type, id);
        }
    }

    initialPage: Page {
        id: root

        actionBar.hidden: true
        anchors.fill: parent

        DropArea {
            property var droppedUrls : undefined;
            property bool inPlaylist;
            anchors.fill: parent;
            onPositionChanged: {
                if(!drag.accepted) return;

                var playlistButton = bPlaylist.mapFromItem(root, drag.x, drag.y);
                var playlistArea = playlist.open ? playlist.mapFromItem(root, drag.x, drag.y) : null;
                if((playlistButton.x > 0 && playlistButton.x < bPlaylist.width &&
                    playlistButton.y > 0 && playlistButton.y < bPlaylist.height) ||
                   playlistArea === null ||
                   (playlistArea.x > 0 && playlistArea.x < playlist.width &&
                    playlistArea.y > 0 && playlistArea.y < playlist.height)) {
                    bPlaylistAttention.openDropZone();
                    inPlaylist = true;
                } else if(bPlaylistAttention.opacity > 0) {
                    bPlaylistAttention.closeDropZone();
                    inPlaylist = false;
                }
            }
            onEntered: {
               if(drag.urls.length > 0) {
                   drag.accept (Qt.CopyAction);
                   droppedUrls = drag.urls;
                   positionChanged(drag);
               } else {
                   drag.accepted = false;
               }
            }
            onDropped: {
               bPlaylistAttention.closeDropZone();
               context.playlist.onFilesDropped(droppedUrls, inPlaylist);
            }
            onExited: {
               bPlaylistAttention.closeDropZone();
            }
        }

        Rectangle {
            id: baseVideo

            color: "black"
            anchors.fill: parent

            property bool basicOsdVisible: false

            VideoOutput {
                //objectName: "videoOutput"
                anchors.fill: parent
                source: mediaPlayer

                MouseArea {
                    enabled: videoBehind

                    property bool hideAsap: false;

                    propagateComposedEvents: true

                    Timer {
                        id: hidder

                        interval: 1500
                        onTriggered: {
                            if(osdOpen || playlist.open) return;

                            baseVideo.basicOsdVisible = false;
                            if(parent.mouseY < root.height - barControls.height) {
                                barControls.showing = false;
                                barControls.peeking = false;
                            }
                            cursorVisible = false;
                            //baseVideo.cursorShape = Qt.BlankCursor;
                        }
                    }

                    Timer {
                        id: showUnlocker
                        interval: 360
                        onTriggered: parent.hideAsap = false;
                    }

                    hoverEnabled: true

                    anchors.fill: parent
                    onDoubleClicked: toggleFullscreen()
                    onClicked: {
                        barControls.peeking = true;
                        barControls.showing = true;
                    }

//                    onMouseXChanged: {
//                        //baseVideo.cursorShape = undefined;
//                        hidder.stop();
//                        baseVideo.basicOsdVisible = true;
//                        barControls.showing = true;
//                        if(mouse.y < root.height - barControls.height * 2) {
//                            barControls.peeking = true;
//                            hidder.start();
//                        } else {
//                            barControls.peeking = false;
//                        }
//                    }
                    onMouseYChanged: {
                        //baseVideo.cursorShape = undefined;
                        hidder.stop();
                        if(osdOpen) return;

                        if(!hideAsap) {
                            baseVideo.basicOsdVisible = true;
                            barControls.showing = true;
                        }
                        if(mouse.y < root.height - barControls.height * 2) {
                            barControls.peeking = true;
                            if(hideAsap) {
                                baseVideo.basicOsdVisible = false;
                                barControls.showing = false;
                                barControls.peeking = false;
                                if(!playlist.open) cursorVisible = false;
                                showUnlocker.start();
                            } else {
                                cursorVisible = true;
                                hidder.start();
                            }
                        } else {
                            cursorVisible = true;
                            barControls.peeking = false;
                            hideAsap = true;
                        }
                    }

                    //onPressed: mouse.accepted = false;
                }
            }

            Text {
                id: lSubtitle
                objectName: "lSubtitle"

                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    bottomMargin: barControls.height + barControls.anchors.bottomMargin + Units.dp(32)
                    margins: Units.dp(32)
                }

                readonly property int ratioSize: Units.dp(1920)
                property real baseFontSize: Units.dp(56)
                font.pixelSize: baseFontSize * window.width / ratioSize
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter

                color: "white"

                style: Text.Outline
                styleColor: "black"
                textFormat: Text.StyledText
            }

            LinearGradient {
                opacity: baseVideo.basicOsdVisible ? (videoBar.open ? (videoBehind && !osdOpen ? 1 : 0) : 0) : 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 340
                    }
                }

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                height: videoBar.height * 3

                cached: true

                start: Qt.point(0, 0)
                end: Qt.point(0, height)

                gradient: Gradient {
                    GradientStop { position: 0.00; color: "#66000000" } // #33
                    GradientStop { position: 0.33; color: "#32000000" } // #19
                    GradientStop { position: 1.00; color: "#00000000" }
                }

                ActionBar {
                    id: videoBar

                    property bool open: true

                    visible: parent.opacity > 0

                    title: context.playlist.mediaTitle
                    // TODO maybe update only when visible? maybe Qt already takes care of that?

//					backAction: Action {
//						name: qsTr("Home");
//						iconName: "action/open_in_browser"
//						onTriggered: openOsd();
//					}

                    actions: [
                        Action {
                            iconName: "navigation/expand_more"
                            name: qsTr("Minimize")
                            onTriggered: visibility = Window.Minimized;
                        },
                        Action {
                            iconName: visibility == Window.FullScreen ? "navigation/fullscreen_exit" : "navigation/fullscreen"
                            name: visibility == Window.FullScreen ? qsTr("Leave Fullscreen") : qsTr("Go Fullscreen")
                            onTriggered: toggleFullscreen();
                        },
                        Action {
                            iconName: "navigation/close"
                            name: qsTr("Close")
                            onTriggered: close();
                        }
                    ]
                }
            }

            z: -100
        }

        Loader {
            id: dialogAddPropertyToLibrary

            function open(dialogTitle, addToGroup, replacePages) {
                if(component) {
                    sourceComponent = component;
                    component = null;
                }
                item.show(dialogTitle, addToGroup, replacePages);
            }

            property Component component: Component {
                InputDialog {
                    property string group;
                    property bool replacePageStack;

                    positiveButtonText: qsTr("Add", "Button in the \"Add artist/tv show/etc\" dialog")
                    negativeButtonText: qsTr("Cancel", "Button in the \"Add artist/tv show/etc\" dialog")

                    validator: RegExpValidator { regExp: /.+/ }
                    textField.onAccepted: {
                        if(value.length > 0) {
                            accepted();
                            close();
                        }
                    }

                    onAccepted: {
                        if(value.length > 0)
                            libraryList.openInfoPage(libraryModel.registerItem(group, value), replacePageStack);
                    }

                    function show(dialogTitle, addToGroup, replacePages) {
                        textField.text = "";
                        title = dialogTitle;
                        group = addToGroup;
                        replacePageStack = replacePages;
                        open();
                        forceFocus.start();
                    }

                    Timer {
                        id: forceFocus
                        interval: 50
                        onTriggered: textField.forceActiveFocus()
                    }
                }
            }
        }
        Loader {
            id: dialogRenameMediaProperty

            function open(type, id) {
                if(component) {
                    sourceComponent = component;
                    component = null;
                }
                item.show(type, id);
            }

            property Component component: Component {
                InputDialog {
                    property string itemType;
                    property int itemId;

                    title: qsTr("Rename")

                    positiveButtonText: qsTr("Save changes", "Button in the \"Rename artist/tv show/etc\" dialog")
                    negativeButtonText: qsTr("Cancel", "Button in the \"Rename artist/tv show/etc\" dialog")

                    validator: RegExpValidator { regExp: /.+/ }
                    textField.onAccepted: {
                        if(value.length > 0) {
                            accepted();
                            close();
                        }
                    }

                    onAccepted: {
                        if(value.length > 0)
                            libraryModel.renameMediaProperty(itemType, itemId, value);
                    }

                    function show(type, id) {
                        textField.text = libraryModel.getMediaPropertyName(type, id);
                        itemType = type;
                        itemId = id;
                        open();
                        forceFocus.start();
                    }

                    Timer {
                        id: forceFocus
                        interval: 50
                        onTriggered: { textField.forceActiveFocus(); textField.selectAll(); }
                    }
                }
            }
        }

        SidePanel {
            id: library
            open: context.settings.librarySidebarOpen

            userOpened: userOpenedOsd
            osdMode: videoBehind

            anchors.bottom: videoBehind ? parent.bottom : barControls.top

            width: Math.round(Units.gridUnit * (window.width > Units.dp(1024) ? 4 : 3.5))

            ListView {
                id: libraryList

                Scrollbar {
                    flickableItem: parent
                }

                bottomMargin: videoBehind ? barControls.height : 0

                PopupMenu {
                    id: libraryGroupPopupMenu

                    width: library.width

                    property int index;
                    property string itemName: "";
                    property InputDialog registerMediaPropertyDialog: null;

                    actions: [ Action {
                            iconName: "content/add"
                            name: qsTr("Add %1").arg(libraryGroupPopupMenu.itemName)
                            onTriggered: {
                                dialogAddPropertyToLibrary.open(name, libraryModel.getInternalGroupName(libraryGroupPopupMenu.index), true);
//                                if(libraryGroupPopupMenu.registerMediaPropertyDialog == null) {
//                                    libraryGroupPopupMenu.registerMediaPropertyDialog = Qt.createQmlObject('import Material 0.2; InputDialog {positiveButtonText: qsTr("Add")}', libraryGroupPopupMenu);
//                                    libraryGroupPopupMenu.registerMediaPropertyDialog.textField.onAccepted.connect(function() {
//                                        if(libraryGroupPopupMenu.registerMediaPropertyDialog.value.length > 0) {
//                                            libraryGroupPopupMenu.registerMediaPropertyDialog.accepted();
//                                            libraryGroupPopupMenu.registerMediaPropertyDialog.close();
//                                        }
//                                    });
//                                    libraryGroupPopupMenu.registerMediaPropertyDialog.onAccepted.connect(function() {
//                                        if(libraryGroupPopupMenu.registerMediaPropertyDialog.value.length > 0)
//                                            libraryList.openInfoPage(libraryModel.registerItem(libraryGroupPopupMenu.index, libraryGroupPopupMenu.registerMediaPropertyDialog.value));
//                                    });
//                                    libraryGroupPopupMenu.registerMediaPropertyDialog.onOpened.connect(function() {
//                                        libraryGroupPopupMenu.registerMediaPropertyDialog.textField.forceActiveFocus();
//                                    });
//                                }

//                                var dialog = libraryGroupPopupMenu.registerMediaPropertyDialog;
//                                dialog.textField.text = "";
//                                dialog.title = name;
//                                dialog.show();
                            }
                        }
                    ]
                }
                PopupMenu {
                    id: libraryItemPopupMenu

                    width: library.width

                    property int index;
                    property string itemName: "";
                    property string itemType;
                    property int itemId;

                    actions: [ Action {
                            iconName: "av/play_arrow"
                            name: qsTr("Play")
                            onTriggered: libraryModel.playAll(libraryItemPopupMenu.index);
                        }, Action {
                            iconName: "av/play_arrow"
                            name: qsTr("Play after this one")
                            enabled: false
                        }, Action {
                            iconName: "content/create"
                            name: qsTr("Rename")
                            onTriggered: uiState.openRenameMediaPropertyDialog(libraryItemPopupMenu.itemType, libraryItemPopupMenu.itemId);
                        }, Action {
                            iconName: "content/remove"
                            name: qsTr("Remove “%1” from library").arg(libraryItemPopupMenu.itemName)
                            onTriggered: {
                                showMediaPropertyRemovalSnackbar(libraryItemPopupMenu.itemName);
                                libraryModel.removeFromLibrary(libraryItemPopupMenu.index);
                            }
                        }
                    ]
                }

                model: libraryModel

                delegate: LibraryListItem {
                    id: libraryListItem
                    itemType: type
                    text: title
                    //valueText: "1"
                    //subText: type == 0 ? "" : "test"
                    iconName: type == 0 ? image : ""
                    //height: type == 0 ? Units.dp(48) : Units.dp(40)

                    onClicked: {
                        if(type == 0) libraryList.openList(title, libraryModel.getListUiAt(i));
                        else if(type == 1) libraryList.openInfoPage(libraryModel.getItem(i), true);
                    }
                    onOpenContextMenu: {
                        if(type == 0) {
                            libraryGroupPopupMenu.index = i;
                            libraryGroupPopupMenu.itemName = libraryModel.getSingularItemName(i);
                            libraryGroupPopupMenu.open(libraryListItem, 0, libraryListItem.height);
                        } else {
                            libraryItemPopupMenu.index = i;
                            libraryItemPopupMenu.itemName = title;
                            libraryItemPopupMenu.itemId = id;
                            libraryItemPopupMenu.itemType = mediaType;
                            libraryItemPopupMenu.open(libraryListItem, 0, libraryListItem.height);
                        }
                    }
                }

                function openList(title, model) {
                    var page = navigator.openPage("MediaPropertyList", true);
                    page.title = title;
                    page.ui = model;
                }

                function openInfoPage(model, replace) {
                    if(model !== null) navigator.openPage("MediaContentPage", replace).ui = model;
                }

                anchors {
                    left: parent.left
                    top: library.toolbar/*Library*/.bottom
                    right: parent.right
                    bottom: parent.bottom
                }

                add: Transition {
                    ParallelAnimation {
                        NumberAnimation { property: "x"; from: -Units.gridUnit; duration: 260; easing.type: Easing.InOutCubic }
                        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 240}
                    }
                }
                remove: Transition {
                    ParallelAnimation {
                        NumberAnimation { property: "x"; to: -Units.gridUnit; duration: 260; easing.type: Easing.InOutCubic }
                        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 240}
                    }
                }

                addDisplaced: Transition {
                    id: llada
                    SequentialAnimation {
                        PauseAnimation {
                            duration: Math.abs(llada.ViewTransition.index -
                                    llada.ViewTransition.targetIndexes[0]) * 25
                        }
                        NumberAnimation { properties: "y"; duration: 220; easing.type: Easing.InOutCubic }
                    }
                }
                removeDisplaced: Transition {
                    id: llrda
                    SequentialAnimation {
                        PauseAnimation {
                            duration: Math.abs(llrda.ViewTransition.index -
                                    llrda.ViewTransition.targetIndexes[0]) * 25 + 220
                        }
                        NumberAnimation { properties: "y"; duration: 220; easing.type: Easing.InOutCubic }
                    }
                }
            }

            title: qsTr("Library")

            maxActionCount: 1

            backAction: Action {
                iconName: "navigation/chevron_left"
                //name: qsTr("Hide") // BUG Material library (or QML's MouseArea?) isn't handling tooltips well, better remove this one
                onTriggered: {
                    hideLibrary()
                    library.wasOpen = false;
                    context.settings.librarySidebarOpen = false;
                }
            }

            actions: [ Action {
                        iconName: "navigation/refresh"
                        name: qsTr("Refresh library")
                        onTriggered: {
                            if(context.isLibraryBeingRefreshed()) {
                                snackbar.show(qsTr("The library is already being refreshed. Your music and TV shows will show up shortly. You can help it by manually adding your favorite artists and TV shows."), 8000);
                            } else {
                                context.refreshLibrary();
                            }
                        }
                    }, Action {
                        iconName: "action/settings"
                        name: qsTr("Library settings")
                        onTriggered: openSettings("library")
                    } ]
        }

        View {
            elevation: videoBehind ? 0 : 3
            Behavior on elevation {
				NumberAnimation {
					duration: 280
					easing.type: Easing.InOutQuad
				}
			}

            backgroundColor: videoBehind ? "#00000000" : "#eeeeee"

            anchors {
                left: library.right
                top: parent.top
                right: parent.right // playlist.left
                bottom: videoBehind ? parent.bottom : barControls.top
            }

            /*Navigator*/PageStack {
                id: navigator

//                toolbar: toolbarMain
//                rightPanel: playlist
//                uiState: uiState

                opacity: videoBehind ? (osdOpen ? 0.9 : 0) : 1
                visible: opacity != 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 320
                    }
                }

                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right // playlist.left
                    bottom: parent.bottom
                }

//                Component.onCompleted: {
//                    openPage("HomePage", false, {"ui": homeUiProvider});
//                }

                function openPage(pageName, replace, extraProperties) {
                    var component = Qt.createComponent("pages/" + pageName + ".qml");
                    if (component && component.status === Component.Ready) {
                        var properties = {"navigator": navigator,
                            "toolbar": toolbarMain,
                            "rightPanel": playlist,
                            "uiState": uiState};
                        if(extraProperties) {
                            for(var name in extraProperties) {
                                properties[name] = extraProperties[name];
                            }
                        }
                        var page = component.createObject(navigator, properties);
//                        page.navigator = navigator;
//                        page.toolbar = toolbarMain;
//                        page.rightPanel = playlist;
//                        page.uiState = uiState;

                        if(replace === true) {
                            while(navigator.depth > 1) navigator.pop();
                        }

                        //push(page);
                        push({item:page, destroyOnPop:true});

                        return page;
                    } else {
                        console.log("Couldn't load \"../pages/" + pageName + ".qml\": " + component.errorString());
                    }

                    return null;
                }
            }

            View {
                id: playlist

                Scrollbar {
                    flickableItem: lvPlaylist
                }

                property bool open: context.settings.playlistSidebarOpen
                property bool wasOpen: true
                property bool osdMode: videoBehind

                property bool floatingCard: videoBehind && !osdOpen

                onOsdModeChanged: {
                    if(osdMode && !userOpenedOsd) {
                        wasOpen = open;
                        open = false;
                    } else {
                        if(!open) open = wasOpen;
                    }
                }

                //osdMode: videoBehind
                //leftSide: false
                //color: videoBehind ? "#44ffffff" : "#ffffff"
                backgroundColor: videoBehind ? "#88ffffff" : "#ffffff"

                anchors.rightMargin: open ? (floatingCard ? Units.dp(16) : (toolbarMain.bigPictureMode ? Units.dp(4) : 0)) : -width
                anchors.right: parent.right
                anchors.top: parent.top // toolbarMain.bottom
                anchors.topMargin: toolbarMain.height // floatingCard ? Units.gridUnit : 0
                anchors.bottom: parent.bottom // videoBehind ? parent.bottom : barControls.top
                anchors.bottomMargin: floatingCard ? barControls.anchors.bottomMargin + barControls.height + Units.dp(16) : (toolbarMain.bigPictureMode ? Units.dp(4) : 0)

                transitions: Transition {
                    AnchorAnimation { duration: 300; easing.type: Easing.InOutCubic }
                }
                Behavior on anchors.bottomMargin {
                    NumberAnimation { duration: 260; easing.type: Easing.InOutCubic }
                }

                width: Math.round(Units.gridUnit * (window.width > Units.dp(1024) ? 4 : 3.5))

                elevation: floatingCard | toolbarMain.bigPictureMode ? 1 : 0
                radius: floatingCard | toolbarMain.bigPictureMode ? Units.dp(1) : 0

                Behavior on anchors.rightMargin {
                    NumberAnimation {
                        duration: 360
                        easing.type: Easing.InOutCubic
                    }
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom

                    width: 1

                    color: Qt.rgba(0,0,0,0.1)
                }

                    ListView {
                        id: lvPlaylist
                        z: 1

                        opacity: lSavedPlaylists.opacity == 1 ? 0.1 : 1
                        Behavior on opacity { NumberAnimation { duration: 120 } }

                        headerPositioning: ListView.PullBackHeader
                        header: Rectangle {
                            id: playlistHeader
                            z: 2
                            color: videoBehind && lvPlaylist.atYBeginning ? "#00ffffff" : "#66ffffff"
                            Behavior on color { ColorAnimation { duration: 160 } }

                            width: lvPlaylist.width
                            height: Units.gu(1)

                            property string imageUrl: context.playlist.imageUrl
                            property string lastImageUrl: "";
                            property bool hasPicture: imageUrl.length > 0

                            onImageUrlChanged: { if(imageUrl.length > 0) lastImageUrl = imageUrl; }

                            Item {
                                opacity: playlistHeader.hasPicture ? 1 : 0
                                Behavior on opacity { NumberAnimation { duration: 240 } }
                                anchors.fill: parent
                                clip: true

                                Image {
                                    id: background
                                    anchors.fill: parent
                                    source: parent.opacity > 0 ? (playlistHeader.hasPicture ? playlistHeader.imageUrl : playlistHeader.lastImageUrl) : ""
                                    sourceSize.width: width
                                    fillMode: Image.PreserveAspectCrop
                                    anchors.bottomMargin: -Units.dp(8)
//                                    transform: playlistHeader.hasPicture ? normal : moving

//                                    Translate { id: normal; y: 0 }
//                                    Translate { id: moving; y: Units.dp(16) }
                                    transform: Translate {
                                        id: pictureTranslation
                                        y: playlistHeader.hasPicture ? -Units.dp(8) : 0
                                        Behavior on y { NumberAnimation { duration: 300 } }
                                    }

                                    //Behavior on y { NumberAnimation { duration: 280 } }

//                                    anchors.topMargin: playlistHeader.hasPicture ? -Units.dp(16) : 0
//                                    anchors.bottomMargin: playlistHeader.hasPicture ? -Units.dp(16) : 0

//                                    Behavior on anchors.topMargin { NumberAnimation { duration: 280; easing.type: Easing.InOutCubic } }
//                                    Behavior on anchors.bottomMargin { NumberAnimation { duration: 280; easing.type: Easing.InOutCubic } }


                                }

                                LinearGradient {
                                    anchors.fill: parent

                                    cached: true

                                    start: Qt.point(0, 0)
                                    end: Qt.point(0, height)

                                    gradient: Gradient {
                                        GradientStop { position: 1.00; color: "#77000000" } // #33
                                        GradientStop { position: 0.33; color: "#33000000" } // #19
                                        GradientStop { position: 0.00; color: "#11000000" }
                                    }
                                }
                            }

                            Label {
                                id: tPlaylistTitle

                                anchors {
                                    left: parent.left
                                    leftMargin: Units.dp(8)
                                    right: parent.right
                                    rightMargin: Units.dp(48)
//                                    bottom: tPlaylistDescription.text.length == 0 ? undefined :  parent.verticalCenter
//                                    verticalCenter: tPlaylistDescription.text.length == 0 ?  parent.verticalCenter : undefined
                                }

                                opacity: tfPlaylistTitle.activeFocus ? 0 : 1
                                Behavior on opacity { NumberAnimation { duration: 180 } }

                                elide: Text.ElideRight

                                states: [
                                    State {
                                        when: tPlaylistDescription.actualText.length == 0
                                        AnchorChanges {
                                            target: tPlaylistTitle
                                            anchors.verticalCenter: tPlaylistTitle.parent.verticalCenter
                                            anchors.bottom: undefined
                                        }
                                    }, State {
                                        when: tPlaylistDescription.actualText.length != 0
                                        AnchorChanges {
                                            target: tPlaylistTitle
                                            anchors.bottom: tPlaylistTitle.parent.verticalCenter
                                            anchors.verticalCenter: undefined
                                        }
                                        PropertyChanges {
                                            target: tPlaylistTitle
                                            anchors.bottomMargin: (tPlaylistDescription.height - tPlaylistTitle.height) / 4
                                        }
                                    }]

                                transitions: Transition {
                                    AnchorAnimation { duration: 300; easing.type: Easing.InOutCubic/*Bezier; easing.bezierCurve: [0.4, 0.0, 0.8, 1]*/ }
                                }

                                style: "title"
                                text: context.playlist.title // qsTr("Playlist")
                                color: playlistHeader.hasPicture ? Theme.dark.textColor : Theme.light.textColor
                            }
                            TextField {
                                id: tfPlaylistTitle
                                anchors.fill: tPlaylistTitle
                                font: tPlaylistTitle.font
                                style: MaterialStyle.TextFieldStyle { textColor: tPlaylistTitle.color }
                                text: tPlaylistTitle.text
                                onActiveFocusChanged: text = Qt.binding(function(){return tPlaylistTitle.text})
                                opacity: activeFocus ? 1 : 0
                                Behavior on opacity { NumberAnimation { duration: 180 } }
                                onAccepted: {
                                    if(_context.playlist.hasSourceUrl)
                                        _context.playlist.save(tfPlaylistTitle.text);
                                    else
                                        _context.playlist.saveAs(tfPlaylistTitle.text);
                                    playlist.forceActiveFocus();
                                }
                            }

                            Label {
                                id: tPlaylistDescription

                                anchors {
                                    left: parent.left
                                    leftMargin: Units.dp(8)
                                    right: parent.right
                                    rightMargin: Units.dp(48)
                                    top: tPlaylistTitle.bottom //parent.verticalCenter
                                }

                                style: "body2"
                                property string actualText: context.playlist.description
                                property string visibleText;
                                onActualTextChanged: if(actualText.length > 0) visibleText = actualText;
                                text: visibleText
                                color: playlistHeader.hasPicture ? Theme.dark.textColor : Theme.light.textColor
                                opacity: actualText.length == 0 ? 0 : 1
                                Behavior on opacity {
                                    NumberAnimation { duration: 280 }
                                }
                            }

                            IconButton {
                                width: Units.dp(40)
                                color: playlistHeader.hasPicture ? Theme.dark.iconColor : Theme.light.iconColor
                                anchors {
                                    top: parent.top
                                    bottom: parent.bottom
                                    right: parent.right
                                    rightMargin: Units.dp(8)
                                    //verticalCenter: parent.verticalCenter
                                }

                                action: Action {
                                    // BUG MouseArea's problems...
                                    // text: qsTr("Playlist options")
                                    iconName: "navigation/more_vert"
                                    onTriggered: mPlaylistOptions.open(playlistHeader, 0, playlistHeader.height);
                                }

                                PopupMenu {
                                    id: mPlaylistOptions

                                    width: playlist.width

                                    actions: [
                                        Action {
                                            text: qsTr("Add file")
                                            iconName: "content/add"
                                            onTriggered: askUserToOpenMedia();
                                        }, Action {
                                            text: qsTr("Clear")
                                            iconName: "content/clear"
                                            onTriggered: _context.playlist.clear();
                                        }, Action {
                                            text: qsTr("Open playlist")
                                            iconName: "av/playlist_play"
                                            onTriggered:{
                                                lSavedPlaylists.state = "open";
                                                var savedPlaylists = _context.playlist.getSavedPlaylists();
                                                var len = savedPlaylists.length;
                                                for(var i = 0; i < len; i++) {
                                                    savedPlaylistsModel.append({"playlist":savedPlaylists[i]});
                                                }
                                            }
                                        }, Action {
                                            text: qsTr("Save playlist")
                                            iconName: "av/playlist_add"
                                            onTriggered: {
                                                var closer = function() {
                                                    tfPlaylistTitle.forceActiveFocus();
                                                    mPlaylistOptions.onClosed.disconnect(closer);
                                                };
                                                mPlaylistOptions.onClosed.connect(closer);
                                            }
                                        }
                                    ]
                                }
                            }

                            Card {
                                visible: opacity != 0 //tfPlaylistTitle.activeFocus
                                height: pPlaylistOptions.height
                                //color: "#ffffff"
                                radius: 0

                                opacity: tfPlaylistTitle.activeFocus ? 1 : 0
                                Behavior on opacity { NumberAnimation { duration: 180 } }

                                Column {
                                    id: pPlaylistOptions
                                    anchors {
                                        left: parent.left
                                        right: parent.right
                                    }

                                    Button {
                                        id: bSavePlaylist
                                        visible: _context.playlist.hasSourceUrl
                                        width: parent.width
                                        text: qsTr("Replace saved playlist")
                                        onClicked: { _context.playlist.save(tfPlaylistTitle.text); playlist.forceActiveFocus(); }
                                    }
                                    Button {
                                        id: bSavePlaylistAs
                                        width: parent.width
                                        text: qsTr("Save playlist")
                                        onClicked: { _context.playlist.saveAs(tfPlaylistTitle.text); playlist.forceActiveFocus(); }
                                    }
                                    Button {
                                        width: parent.width
                                        text: qsTr("Cancel")
                                        onClicked: playlist.forceActiveFocus()
                                    }
                                }

                                anchors {
                                    top: parent.top
                                    topMargin: tfPlaylistTitle.activeFocus ? Units.gu(1) : (Units.gu(1) + -Units.dp(16))
                                    Behavior on topMargin { NumberAnimation { duration: 240; easing.type: Easing.InOutCubic } }
                                    left: parent.left
                                    right: parent.right
                                }
                            }
                        }

                        PopupMenu {
                            id: lvPlaylistPopupMenu

                            width: playlist.width

                            property int index;
                            property Item item;
                            property var relatedProperties;

                            actions: [ Action {
                                    iconName: "av/play_arrow"
                                    name: qsTr("Play now")
                                    onTriggered: context.playlist.play(lvPlaylistPopupMenu.index);
                                }, Action {
                                    //iconName: "av/play_arrow"
                                    name: lvPlaylistPopupMenu.index === context.playlist.currentIndex ? qsTr("Repeat once after") : qsTr("Play after this one")
                                    checkable: true
                                    readonly property bool check: lvPlaylistPopupMenu.index === context.playlist.indexToPlayAfter
                                    checked: check;
                                    onTriggered: {
                                        if(checked) context.playlist.indexToPlayAfter = lvPlaylistPopupMenu.index;
                                        else context.playlist.indexToPlayAfter = -1;
                                        checked = Qt.binding(function() { return check; });
                                    }
                                    visible: context.playlist.currentIndex >= 0
                                }, ActionMenu {
                                    iconName: "action/info"
                                    name: qsTr("Info about");
                                    onTriggered: { submenuOpener.source = source; submenuOpener.start(); }
                                    Loader {
                                        id: submenuInfo
                                        asynchronous: true
                                        width: playlist.width
                                        parent: playlist
                                        sourceComponent: PopupMenu {
                                            Component.onCompleted: {
                                                lvPlaylistPopupMenu.onOpened.connect(updateActions);
                                                updateActions();
                                            }

                                            function updateActions() {
                                                var info = lvPlaylistPopupMenu.relatedProperties;
                                                if(info) {
                                                    var actions = [];
                                                    var len = info.ids.length;
                                                    for(var i = 0; i < len; i++) {
                                                        actions[actions.length] = action.createObject(submenuInfo.item, {"name": info.names[i], "propertyId": info.ids[i], "propertyType": info.types[i]});
                                                    }
                                                    submenuInfo.item.actions = actions;
                                                }
                                            }

                                            Component {
                                                id: action
                                                Action {
                                                    property int propertyId
                                                    property string propertyType

                                                    iconName: propertyType == "artist" ? "social/people" : propertyType == "tvShow" ? "hardware/tv" : ""

                                                    onTriggered: {
                                                        if(propertyId == -1) uiState.showSnackbar(qsTr("Information about the song/episode coming in a future version."), 4000);
                                                        else uiState.openMediaPropertyInfoPage(propertyType, propertyId);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    Timer {
                                        id: submenuOpener
                                        property Item source
                                        interval: 20
                                        onTriggered: submenuInfo.item.open(source, 0, -Units.dp(16));
                                    }
                                }/*, Action {
                                    iconName: "action/info"
                                    name: qsTr("Info")
                                    onTriggered: {
                                        var component = Qt.createComponent("pages/MediaContentInfoEditor.qml");
                                        if (component && component.status === Component.Ready) {
                                            var popup = component.createObject(window);
                                            popup.open(lvPlaylistPopupMenu.item);
                                        }
                                    }
                                }*/, Action {
                                    iconName: "content/remove"
                                    name: qsTr("Remove from this list")
                                    onTriggered: context.playlist.removeMedia(lvPlaylistPopupMenu.index);
                                }
                            ]
                        }

                        model: context.playlist

                        delegate: Item {
                            id: playlistItem

                            width: lvPlaylist.width
                            height: root.height < Units.gridUnit * 6 ? Units.dp(40) : Units.dp(48)

                            Item {
                                id: mediaImage
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.height

                                Rectangle {
                                    color: "#22000000"
                                    anchors.fill: parent
                                }

                                Label {
                                    anchors.right: parent.right
                                    anchors.rightMargin: Units.dp(6)
                                    anchors.bottomMargin: Units.dp(6)
                                    anchors.bottom: parent.bottom

                                    text: coverText
                                    style: "caption"
                                    color: Theme.dark.textColor
                                }

                                Icon {
                                    property bool goingToPlayAfter: index >= 0 && context.playlist.indexToPlayAfter === index
                                    visible: !inkPlaylistItem.containsMouse
                                    opacity: index === context.playlist.currentIndex || goingToPlayAfter ? 1 : 0
                                    anchors.centerIn: parent
                                    size: Units.dp(24)
                                    name: goingToPlayAfter ? "content/reply" : (context.playbackState === MediaPlayer.PlayingState ? "av/play_circle_filled" : "av/pause_circle_filled")
                                    Behavior on opacity { NumberAnimation { duration: 100 } }
                                }

                                Icon {
                                    //visible: inkPlaylistItem.containsMouse
                                    opacity: inkPlaylistItem.containsMouse ? 1 : 0
                                    anchors.centerIn: parent
                                    size: Units.dp(24)
                                    anchors.margins: Units.dp(10)
                                    name: index === context.playlist.currentIndex && context.playbackState !== MediaPlayer.StoppedState ? "av/replay" : "av/play_circle_filled"
                                    Behavior on opacity { NumberAnimation { duration: 100 } }
                                }
                            }

//                            Icon {
//                                id: mediaImage
//                                anchors.left: parent.left
//                                anchors.top: parent.top
//                                anchors.bottom: parent.bottom
//                                width: height
//                                name: "av/album"
//                                size: width
//                            }
                            Item {
                                z: 3
                                anchors.left: mediaImage.right
                                anchors.leftMargin: Units.dp(8)
                                anchors.right: parent.right
                                anchors.rightMargin: Units.dp(8)
                                //anchors.verticalCenter: parent.verticalCenter
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                Label {
                                    id: mediaTitle
                                    text: title ? title : mediaName
                                    style: "body2"
                                    color: index === context.playlist.currentIndex ? Theme.primaryColor : Theme.light.textColor
                                    //anchors.top: parent.top
                                    anchors.bottom: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    elide: Text.ElideRight
                                }
                                Loader {
                                    sourceComponent: errorMessage ? errorIconComponent : undefined
                                Component {
                                    id: errorIconComponent
                                IconButton {
                                    parent: mediaTitle.parent
                                    z: 3
                                    visible: errorType != undefined
                                    color: "#EF9A9A"
                                    action: Action {
                                        name: errorMessage
                                        iconName: "alert/error"
                                    }
                                    //name: "alert/error"
                                    size: Units.dp(16)
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.topMargin: Units.dp(8)
//                                    MouseArea { id: errorInfo; z: 4; hoverEnabled: true; anchors.fill: parent }
//                                    Tooltip {
//                                        text: "There was an error loading this: " + error
//                                        mouseArea: errorInfo
//                                    }
                                }}
                                }
                                Label {
                                    id: mediaDescription
                                    anchors.top: parent.verticalCenter // mediaTitle.bottom
                                    anchors.left: parent.left
                                    anchors.right: mediaDuration.left
                                    text: description
                                    color: Theme.light.subTextColor

                                    elide: Text.ElideRight
                                }
                                Label {
                                    id: mediaDuration
                                    anchors.top: mediaDescription.top
                                    anchors.right: parent.right
                                    text: duration
                                    color: Theme.light.subTextColor
                                }
                            }

                            Ink {
                                id: inkPlaylistItem
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: {
                                    if(mouse.button === Qt.LeftButton)
                                        context.playlist.play(index);
                                    else if(mouse.button === Qt.RightButton) {
                                        lvPlaylistPopupMenu.index = index;
                                        lvPlaylistPopupMenu.item = playlistItem;
                                        lvPlaylistPopupMenu.relatedProperties = relatedProperties;
                                        lvPlaylistPopupMenu.open(playlistItem, 0, playlistItem.height);
                                    }
                                }
                            }
                        }

                        anchors {
                            left: parent.left
                            leftMargin: videoBehind ? 0 : 1
                            top: parent.top
                            right: parent.right
                            bottom: parent.bottom // bAddMediaToPlaylist.top
                        }

                        add: Transition {
                            id: lpaa
                            SequentialAnimation {
                                NumberAnimation { property: "opacity"; from: 0; to: 0; duration: 0 }
                                PauseAnimation {
                                    duration: (lpaa.ViewTransition.index -
                                               lpaa.ViewTransition.targetIndexes[0]) * 30
                                }
                                ParallelAnimation {
                                    NumberAnimation { property: "opacity"; from: 0; to: 1; easing.type: Easing.InOutCubic; duration: 360 }
                                    NumberAnimation { property: "x"; from: -Units.dp(32); easing.type: Easing.InOutCubic; duration: 380 }
                                }
                            }
                        }
                        remove: Transition {
                            //id: lpra
                            NumberAnimation { property: "opacity"; to: 0; from: 1; easing.type: Easing.InOutCubic; duration: 260 }
                            NumberAnimation { property: "x"; to: +Units.dp(32); easing.type: Easing.InOutCubic; duration: 280 }
                        }
                        displaced: transitionItemsMoving
                        move: transitionItemsMoving

                        Transition {
                            id: transitionItemsMoving
                            NumberAnimation { property: "y"; duration: 280; easing.type: Easing.InOutCubic }
                        }

                        bottomMargin: videoBehind && osdOpen ? barControls.height : 0

                        Component.onCompleted: {
                            model.rowsInserted.connect(function() {
                                if(!playlist.open)
                                    bPlaylistAttention.requestAttention();
                            });
                        }
                }

                ListView {
                    id: lSavedPlaylists
                    state: "closed"

                    bottomMargin: videoBehind && osdOpen ? barControls.height : 0

                    function close() {
                        lSavedPlaylists.state = "closed";
                        var len = savedPlaylistsModel.count;
                        for(var i = 0; i < len; i++) {
                            if(savedPlaylistsModel.get(i).playlist.toDelete) {
                                savedPlaylistsModel.get(i).playlist.deletePlaylist();
                            }
                        }
                        savedPlaylistsModel.clear();
                    }

                    states: [
                        State {
                            name: "open"
                            PropertyChanges { target: lSavedPlaylists; z: 1 }
                            PropertyChanges { target: lSavedPlaylists; opacity: 1 }
                        },
                        State {
                            name: "closed"
                            PropertyChanges { target: lSavedPlaylists; z: -1 }
                            PropertyChanges { target: lSavedPlaylists; opacity: 0 }
                        }
                    ]
                    transitions: [
                        Transition {
                            from: "closed"
                            to: "open"
                            SequentialAnimation{
                                NumberAnimation { target: lSavedPlaylists; property: "opacity"; duration: 100; }
                            }
                        },
                        Transition {
                            from: "open"
                            to: "closed"
                            SequentialAnimation{
                                PauseAnimation { duration: 140 }
                                NumberAnimation { target: lSavedPlaylists; property: "opacity"; duration: 200; }
                                NumberAnimation { target: lSavedPlaylists; property: "z"; duration: 200; }
                            }
                        }
                    ]

                    anchors {
                        left: parent.left
                        leftMargin: videoBehind ? 0 : 1
                        top: parent.top
                        right: parent.right
                        bottom: parent.bottom // bAddMediaToPlaylist.top
                    }

                    headerPositioning: ListView.OverlayHeader
                    header: Rectangle {
                        z: 4
                        width: lSavedPlaylists.width
                        height: Units.gridUnit
                        color: "#ffffff"

                        Label {
                            anchors {
                                left: parent.left
                                leftMargin: Units.dp(8)
                                right: parent.right
                                rightMargin: Units.dp(48)
                                verticalCenter: parent.verticalCenter
                            }

                            style: "title"
                            text: qsTr("Open playlist")
                        }

                        IconButton {
                            width: Units.dp(40)
                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                                right: parent.right
                                rightMargin: Units.dp(8)
                            }

                            action: Action {
                                // BUG MouseArea's issues, let's remove the tooltip for now
                                //text: qsTr("Cancel")
                                iconName: "navigation/close"
                                onTriggered: {
                                    lSavedPlaylists.close();
                                }
                            }
                        }
                    }

                    model: ListModel { id: savedPlaylistsModel }

                    delegate: Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: lspTitle.height + lspDescription.height + Units.dp(26)

                        Label {
                            style: "subheading"
                            text: qsTr("Playlist deleted")
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: Units.dp(16)
                        }
                        IconButton {
                            visible: playlist.toDelete
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: Units.dp(16)

                            action: Action {
                                name: qsTr("Undo")
                                iconName: "content/undo"
                                onTriggered: playlist.toDelete = false;
                            }
                        }

                        Card {
                            id: cardSavedPlaylist

                            opacity: playlist.toDelete ? 0 : 1
                            anchors.fill: parent
                            anchors.leftMargin: Units.dp(8)
                            anchors.rightMargin: Units.dp(8)
                            anchors.topMargin: Units.dp(2)
                            anchors.bottomMargin: Units.dp(6)

                            transform: Translate {
                                x: playlist.toDelete ? parent.width : 0
                                Behavior on x { NumberAnimation { duration: 340; easing.type: Easing.InOutCubic } }
                            }
                            Behavior on opacity { NumberAnimation { duration: 300; } }

                            Image {
                                id: spBackground
                                readonly property bool hasPicture: status == Image.Ready // playlist.imageUrl.length > 0
                                anchors.fill: parent
                                source: playlist.imageUrl
                                sourceSize.width: width
                                fillMode: Image.PreserveAspectCrop
//                                anchors.bottomMargin: -Units.dp(8)
//                                transform: Translate {
//                                    id: spPictureTranslation
//                                    y: spBackground.status == Image.Ready ? -Units.dp(8) : 0
//                                    Behavior on y { NumberAnimation { duration: 300 } }
//                                }
                            }

                            LinearGradient {
                                visible: spBackground.hasPicture
                                anchors.fill: parent

                                start: Qt.point(0, 0)
                                end: Qt.point(0, height)

                                gradient: Gradient {
                                    GradientStop { position: 1.00; color: "#77000000" } // #33
                                    GradientStop { position: 0.33; color: "#33000000" } // #19
                                    GradientStop { position: 0.00; color: "#11000000" }
                                }
                            }

                            Label {
                                id: lspTitle
                                text: playlist.name
                                color: spBackground.hasPicture ? Theme.dark.textColor : Theme.light.textColor
                                style: "title"
                                anchors {
                                    top: parent.top
                                    left: parent.left
                                    right: parent.right
                                    margins: Units.dp(8)
                                    topMargin: Units.dp(6)
                                }
                            }
                            Label {
                                id: lspDescription
                                text: playlist.description
                                color: spBackground.hasPicture ? Theme.dark.textColor : Theme.light.textColor
                                style: "caption"
                                anchors {
                                    top: lspTitle.bottom
                                    left: parent.left
                                    right: parent.right
                                    margins: Units.dp(8)
                                    topMargin: Units.dp(2)
                                }
                            }

                            Ink {
                                anchors.fill: parent;
                                color: spBackground.hasPicture ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1);
                                onClicked: {
                                    if(_context.playlist.rowCount() > 0) {
                                        _context.playlist.saveTemporaryPlaylist();
                                        snackbarPlaylist.open(qsTr("Previous list replaced"));
                                    }
                                    _context.playlist.open(playlist);
                                    lSavedPlaylists.close();
                                }
                            }

                            IconButton {
                                color: spBackground.hasPicture ? Theme.dark.iconColor : Theme.light.iconColor
                                anchors.bottom: lspDescription.bottom
                                anchors.right: lspDescription.right

                                action: Action {
                                    name: qsTr("Delete")
                                    iconName: "content/clear"
                                    onTriggered: playlist.toDelete = true;
                                }

                                z: 2
                            }
                        }
                    }

                    add: Transition {
                        id: lspaa
                        SequentialAnimation {
                            NumberAnimation { property: "opacity"; to: 0; duration: 0 }
                            PauseAnimation {
                                duration: (lspaa.ViewTransition.index -
                                           lspaa.ViewTransition.targetIndexes[0]) * 50
                            }
                            ParallelAnimation {
                                NumberAnimation { property: "opacity"; from: 0; to: 1; easing.type: Easing.InOutCubic; duration: 360 }
                                NumberAnimation { property: "y"; from: lspaa.ViewTransition.destination.y-Units.dp(16); to: lspaa.ViewTransition.destination.y; easing.type: Easing.InOutCubic; duration: 380 }
                            }
                        }
                    }
                    remove: Transition {
                        id: lspra
                        SequentialAnimation {
                            PauseAnimation {
                                duration: Math.abs(lspra.ViewTransition.index -
                                           lspra.ViewTransition.targetIndexes[0]) * 60
                            }
                            ParallelAnimation {
                                NumberAnimation { property: "opacity"; from: 1; to: 0; easing.type: Easing.InOutCubic; duration: 360 }
                                NumberAnimation { property: "y"; from: lspra.ViewTransition.destination.y; to: lspra.ViewTransition.destination.y-Units.dp(16); easing.type: Easing.InOutCubic; duration: 400 }
                            }
                        }
                    }
                }

                Loader {
                    id: snackbarPlaylist

                    function open(text) {
                        sourceComponent = _snackbarPlaylist;
                        item.open(text);
                    }

                    Component {
                        id: _snackbarPlaylist
                    Snackbar {
                        id: __snackbarPlaylist
                        z: 10
                        parent: playlist
                        duration: 6500
                        buttonText: qsTr("Join both")
                        onClicked: { _context.playlist.restoreTemporaryPlaylist(); __snackbarPlaylist.opened = false; }
                    }
                    }
                }
            }

            ActualToolbar {
                id: toolbarMain

                opacity: videoBehind ? (osdOpen ? 0.9 : 0) : 1
                Behavior on opacity {
					NumberAnimation {
						duration: 280
					}
                }

                property color textColor: Theme.isDarkColor(toolbarMain.backgroundColor) ? Theme.dark.textColor : Theme.light.textColor
                property bool bigPictureMode: navigator.currentItem ? navigator.currentItem.bigPictureMode : false;
                backgroundOpacity: bigPictureMode ? 0 : 1
                elevation: bigPictureMode ? 0 : 3
                height: navigator.currentItem ? (navigator.currentItem.toolbarHeight === -1 ? Units.gridUnit : navigator.currentItem.toolbarHeight) : Units.gridUnit

                customContent: Item {
                    id: navigationBarItem
                    height: Units.gridUnit
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: -Units.dp(8)

                    TextField {
                        id: tSearchBox
                        readonly property bool open: activeFocus // false
                        readonly property int centerPos: Math.max(parent.parent.x, window.width / 2 - width / 2 - parent.parent.x - toolbarMain.parent.parent.x)
                        property int displacement: open | navigator.depth <= 1 ? 0 : window.width/4
                        x: centerPos + displacement
                        width: window.width/3 // Units.dp(240)
                        anchors.verticalCenter: parent.verticalCenter

                        opacity: open | navigator.depth <= 1 ? 1 : 0

                        Behavior on displacement { NumberAnimation { duration: 280; easing.type: Easing.InOutCubic } }
                        Behavior on opacity { NumberAnimation { duration: 260 } }

                        placeholderText: qsTr("Search")
                        showBorder: false

                        onTextChanged: {
                            var isSearch = navigator.__currentItem.isSearch;
                            if(isSearch) {
                                if(text.length == 0) {
                                    navigator.pop();
                                    tSearchBox.forceActiveFocus();
                                } else {
                                    navigator.__currentItem.search(text);
                                }
                            } else {
                                if(text.length > 0) {
                                    var pSearch = navigator.openPage("SearchPage");
                                    pSearch.search(text);
                                    tSearchBox.forceActiveFocus();
                                }
                            }
                        }

                        IconButton {
                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                                right: parent.right
                            }

                            action: Action {
                                text: qsTr("Close search")
                                iconName: "navigation/close"
                                onTriggered: tSearchBox.text = "";
                            }

                            color: Theme.dark.iconColor

                            opacity: parent.text.length > 0 ? 0.7 : 0
                            Behavior on opacity { NumberAnimation { duration: 200 } }
                        }

                        style: MaterialStyle.TextFieldStyle {
                            padding.right: Units.dp(28)
                            textColor: Theme.dark.textColor //Theme.isDarkColor(toolbarMain.backgroundColor) ? Theme.dark.textColor : Theme.light.textColor
                            background: Item {
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.leftMargin: -Units.dp(6)
                                    anchors.rightMargin: -Units.dp(6)
                                    anchors.topMargin: -Units.dp(2)
                                    anchors.bottomMargin: -Units.dp(2)
                                    color: Theme.isDarkColor(toolbarMain.backgroundColor) ? "#44ffffff" : "#44000000"
                                    Behavior on color {
                                        ColorAnimation { duration: 300 }
                                    }

                                    radius: Units.dp(2)
                                }
                                Label {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: control.placeholderText
                                    color: tSearchBox.textColor
                                    font.pixelSize: Units.dp(16)
                                    opacity: control.text.length > 0 ? 0 : 0.8
                                    Behavior on opacity { NumberAnimation { duration: 140 } }
                                }
                            }
                        }
                    }

                    Row {
                        add: Transition {
                            ParallelAnimation {
                                //NumberAnimation { properties: "x"; from: Units.dp(8); duration: 200 }
                                NumberAnimation { properties: "opacity"; from: 0; to: 1; duration: 180 }
                            }
                        }
//                        anchors.verticalCenter: parent.verticalCenter

                        visible: navigator.depth > 1 && !tSearchBox.open
//                        onVisibleChanged: {
//                            if(visible) tSearchBox.open = false;
//                        }

                        Repeater {
                            model: toolbarMain.bigPictureMode ? navigator.depth-1 : navigator.depth

                            delegate: View {
                                width: label.implicitWidth + Units.dp(40) // 8 left side + 14 text~arrow + 24 arrow
                                height: navigationBarItem.height

                                Label {
                                    id: label

                                    anchors {
                                        top: parent.top
                                        bottom: parent.bottom
                                        left: parent.left
                                        leftMargin: Units.dp(8)
                                    }

                                    text: navigator.get(index).title
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    color: toolbarMain.textColor
                                    style: "title"
                                }

                                Icon {
                                    id: arrow
                                    visible: index+1 != navigator.depth

                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        right: parent.right
                                    }

                                    name: "navigation/chevron_right"
                                    size: Units.dp(24)
                                    color: toolbarMain.textColor
                                    opacity: 0.6
                                }

                                Ink {
                                    id: mouseArea
                                    enabled: index+1 != navigator.depth

                                    anchors.fill: parent
//                                    focused: control.focus && background.context !== "dialog"
//                                            && background.context !== "snackbar"
//                                    focusWidth: parent.width - Units.dp(30)
//                                    focusColor: Qt.darker(background.backgroundColor, 1.05)

                                    onClicked: { navigator.pop(navigator.get(index)); }
                                }
                            }
                        }
                    }
                }

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: !osdOpen && videoBehind ? -height : 0
                Behavior on anchors.topMargin {
                    NumberAnimation {
                        duration: 320
                    }
                }

                backAction: Action {
                    iconName: "navigation/menu"
                    name: qsTr("Library")
                    visible: !library.open; // library.anchors.leftMargin == -library.width
                    onTriggered: {
                        showLibrary()
                        library.wasOpen = true;
                        context.settings.librarySidebarOpen = true;
                    }
                }

                actions: [
//					Action {
//						iconName: "action/settings_overscan" // "action/aspect_ratio"
//						name: "Back to the Video"
//						visible: osdOpen
//						onTriggered: closeOsd();
//					},
                    Action {
                        iconName: "action/search"
                        name: qsTr("Search")
                        visible: tSearchBox.opacity == 0
                        onTriggered: {
                            tSearchBox.forceActiveFocus();
                            tSearchBox.selectAll();
                        }
                    },
                    Action {
                        iconName: "action/settings"
                        name: qsTr("Settings")
                        onTriggered: openSettings()
                    }
//                     , Action {
//                         iconName: "av/queue_music"
//                         name: "Show playlist"
//                         visible: playlist.anchors.rightMargin < 0 //== -playlist.width
//                         onTriggered: showPlaylist()
//                     }
				]

                maxActionCount: 2
            }
        }

        View {
            id: barControls

            backgroundColor: videoBehind ? Qt.rgba(.45, .45, .45, .30) : "#ffffff"

            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            anchors.bottomMargin: !videoBehind | osdOpen ? 0 : showing ? (peeking ? -height + mediaProgress.height + lTime.height + Units.dp(7) : 0) : -height
            Behavior on anchors.bottomMargin {
                NumberAnimation {
                    duration: 400
                    easing.type: Easing.InOutCubic
                }
            }

            height: Units.gridUnit * 1.5

            elevation: 4
            z: 100

            //clip: false
            clipContent: false

            property color iconColor: videoBehind ? Theme.dark.iconColor : Theme.light.iconColor
            property bool showing: true;
            property bool peeking: false;

            Label {
				objectName: "lLength"
				text: "00:00"
				style: "body2"
				//opacity: 0.70
                color: videoBehind ? Theme.dark.textColor : Theme.light.subTextColor
				anchors.right: parent.right
				anchors.rightMargin: Units.dp(8)
				anchors.top: mediaProgress.bottom
                anchors.topMargin: context.isSeekable ? Units.dp(2) : 0
                opacity: context.isSeekable ? 1 : 0
                Behavior on anchors.topMargin {
                    NumberAnimation { duration: 240 }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 240 }
                }
            }

            Label {
                id: lTime
				objectName: "lTime"
				text: "00:00"
				style: "body2"
				//opacity: 0.70
                color: videoBehind ? Theme.dark.textColor : Theme.light.subTextColor
				anchors.left: parent.left
				anchors.leftMargin: Units.dp(8)
                anchors.top: mediaProgress.bottom
                anchors.topMargin: context.isSeekable ? Units.dp(2) : 0
                opacity: context.isSeekable ? 1 : 0
                Behavior on anchors.topMargin {
                    NumberAnimation { duration: 240 }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 240 }
                }
            }

            Item {
                anchors.fill: parent
                anchors.topMargin: Units.dp(4)

				Item {
                    id: pVolume
					width: library.width

					anchors {
						left: parent.left
						top: parent.top
                        //topMargin: Units.dp(4)
						bottom: parent.bottom
					}

//					MouseArea {
//						id: maVolume

//						hoverEnabled: true

//						anchors {
//							left: bMute.left
//							top: bMute.top
//							right: bMute.right
//							bottom: bMute.bottom
//						}
//					}

					IconButton {
						id: bMute

                        property int oldVolume: 0

						anchors.left: parent.left
						anchors.leftMargin: Units.dp(32)
						anchors.verticalCenter: parent.verticalCenter

						iconName: "av/volume_down"

						color: barControls.iconColor

                        onClicked: {
                            if(sVolume.value == 0) {
                                if(oldVolume == 0) {
//                                    smoothVolumeChanger.to = 100;
                                    sVolume.value = 100;
                                } else {
//                                    smoothVolumeChanger.to = oldVolume;
                                    sVolume.value = oldVolume;
                                }
//                                smoothVolumeChanger.start();
                            } else {
                                oldVolume = sVolume.value;
                                sVolume.value = 0;
                            }
                        }

//                        Timer {
//                            id: smoothVolumeChanger
//                            interval: 20
//                            repeat: true

//                            property int to

//                            onTriggered: {
//                                if(Math.abs(sVolume.value - to) >= 2) {
//                                    sVolume.value += (to - sVolume.value) / 10;
//                                } else {
//                                    sVolume.value = to;
//                                    stop();
//                                }
//                            }
//                        }
					}

					Slider {
						id: sVolume

						anchors.left: bMute.right
						anchors.leftMargin: Units.dp(8)
						anchors.right: bMaxVolume.left
						anchors.rightMargin: Units.dp(8)
						//anchors.verticalCenter: parent.verticalCenter
						anchors.bottom: bMute.bottom
						anchors.bottomMargin: Units.dp(2)

						numericValueLabel: true

                        color: Theme.accentColor

						//knobDiameter: Units.dp(24)

						minimumValue: 0
						maximumValue: 100

                        value: 100

						clip: false

                        onValueChanged: onVolumeChanged(value)

						//opacity: hovered || maVolume.containsMouse ? 1 : 0.1

						Behavior on opacity {
							NumberAnimation {
								duration: 200
							}
						}
					}

					Icon {
						id: bMaxVolume

						anchors.right: parent.right
						anchors.rightMargin: Units.dp(32)
						anchors.verticalCenter: parent.verticalCenter

						name: "av/volume_up"

						color: barControls.iconColor
                    }
                }

                IconButton {
                    id: bSubtitles

                    opacity: videoBehind ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 240 } }

                    anchors.left: pVolume.right
                    anchors.leftMargin: library.open && window.width > Units.dp(1024) ? Units.dp(24) : -Units.dp(16)
                    anchors.verticalCenter: pVolume.verticalCenter

                    Behavior on anchors.leftMargin { NumberAnimation { duration: 260 } }

                    color: barControls.iconColor

                    action: Action {
                        id: aSubtitles
                        name: qsTr("Subtitles")
                        iconName: "av/subtitles"
                        onTriggered: pSubtitles.open();
                    }

                    Loader {
                        id: pSubtitles

                        function open() {
                            if(status == Loader.Null) {
                                source = "dialogs/SubtitleSettingsDialog.qml";
                                item.subtitlesEnabled = ui.subtitlesEnabled;
                                item.subtitleSize = lSubtitle.baseFontSize / Units.dp(1);
                                item.subtitleDelay = ui.subtitleDelay;
                            }

                            item.open(bSubtitles, item.width/2);
                            item.onSubtitlesEnabledChanged.connect(function() {
                                ui.subtitlesEnabled = item.subtitlesEnabled;
                            });
                            item.onSubtitleSizeChanged.connect(function() {
                                if(lSubtitle.text.length == 0) lSubtitle.text = qsTr("Subtitles will look like this.");
                                lSubtitle.baseFontSize = Units.dp(item.subtitleSize);
                            });
                            item.onSubtitleDelayChanged.connect(function() {
                                ui.subtitleDelay = item.subtitleDelay;
                            });
                            item.onSubtitleFileOpened.connect(function(file) {
                                ui.loadSubtitlesFromFile(file);
                            });
                        }
                    }
                }

				FloatingActionButton {
					id: bPlay

                    z: 10

                    property real speed: context.playbackRate

                    opacity: barControls.anchors.bottomMargin < -barControls.height/2 ? 0 : 1
                    Behavior on opacity { NumberAnimation { duration: 120 } }

                    //showTooltip: false
                    anchors.centerIn: parent
                    //anchors.horizontalCenter: parent.horizontalCenter
                    //anchors.top: parent.top
                    //anchors.topMargin: Units.dp(4) + parent.height/2 - height/2
					//anchors.topMargin: Units.dp(8) // mediaProgress.height // Units.dp(2)

					action: Action {
                        iconName: "av/play_arrow"
                        name: qsTr("Play") + " (" + qsTr("long press to change speed") + ")"
					}
                    alternateAction: Action {
                        iconName: "av/pause"
                        name: qsTr("Pause") + " (" + qsTr("long press to change speed") + ")"
                    }

                    showAlternateAction: context.playbackState === MediaPlayer.PlayingState

                    animationDuration: 260 / context.playbackRate

					onClicked: {
                        //mediaProgress.value = context.mediaPosition
                        if(context.playbackState === MediaPlayer.PlayingState) {
							context.pause();
						} else if(context.hasMediaToPlay) {
							context.play();
						} else {
							askUserToOpenMedia();
						}
					}

                    drag.target: cursor
                    drag.axis: Drag.XAxis
                    drag.minimumX: 0 //-cursor.radius
                    drag.maximumX: popupSpeedSelectorPanel.width - cursor.width
                    drag.smoothed: false

                    onPressAndHold: {
                        popupSpeedSelector.open(bPlay, 0, 0);
                    }

                    onReleased: {
                        if (popupSpeedSelector.showing) {
                            popupSpeedSelector.close();
                            if(context.playbackRate !== speed) context.playbackRate = speed;
                            else clicked();
                        }
                    }

                    Popover {
                        id: popupSpeedSelector

                        overlayLayer: "tooltipOverlayLayer"
                        globalMouseAreaEnabled: false
                        closeOnResize: false

                        width: /*tooltipLabel.paintedWidth +*/ Units.gridUnit * 5 // Units.dp(192)
                        //implicitHeight: Device.isMobile ? Units.dp(44) : Units.dp(40)
                        implicitHeight: tooltipLabel.implicitHeight + cursor.height + Units.dp(34)

                        backgroundColor: Qt.rgba(0.2, 0.2, 0.2, 0.9)

//                        Item {
//                            Label {
//                                style: "tooltip"
//                                text: qsTr("Speed")
//                                color: Theme.dark.textColor
//                            }
//                        }

                        Item {
                            id: popupSpeedSelectorPanel

                            anchors {
                                fill: parent
                                leftMargin: Units.dp(16)
                                rightMargin: anchors.leftMargin
                                topMargin: Units.dp(12)
                                //bottomMargin: Units.dp(12)
                            }

                            Label {
                                id: tooltipLabel
                                style: "tooltip"
                                text: qsTr("Speed")
                                color: Theme.dark.textColor
                                //anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                            }
                            Label {
                                style: "tooltip"
                                text: bPlay.speed
                                color: Theme.dark.textColor
                                //anchors.verticalCenter: tooltipLabel.verticalCenter
                                anchors.bottom: tooltipLabel.bottom
                                anchors.right: parent.right
                            }

                            Label {
                                style: "tooltip"
                                text: "-"
                                color: Theme.dark.textColor
                                anchors.verticalCenter: cursor.verticalCenter
                                anchors.left: parent.left
                            }
                            Label {
                                style: "tooltip"
                                text: "+"
                                color: Theme.dark.textColor
                                anchors.verticalCenter: cursor.verticalCenter
                                anchors.right: parent.right
                            }

                            Rectangle {
                                id: cursor

                                width: Units.dp(16)
                                height: width
                                radius: width/2

                                color: Theme.accentColor

                                anchors.horizontalCenter: bPlay.drag.active ? undefined : parent.horizontalCenter
                                anchors.top: tooltipLabel.bottom
                                anchors.topMargin: Units.dp(8)
                                //anchors.verticalCenter: parent.verticalCenter

//                                Behavior on x {
//                                    NumberAnimation { duration: 200 }
//                                }

                                onXChanged: {
                                    if(bPlay.drag.active) {
                                        var w = parent.width - width
                                        var a = x / w - 0.5;
                                        var rate = context.playbackRate;
                                        rate = Math.round((rate + (a > 0 ? a*2 : a) * rate) * 20) / 20;
                                        bPlay.speed = rate;
                                    }
                                }
                            }
                        }
                    }
				}

                IconButton {
                    id: bPrevious

                    //anchors.top: bPlay.top
                    //anchors.topMargin: bPlay.height/2 - height/2
                    anchors.verticalCenter: bPlay.verticalCenter
                    anchors.right: bReplay.left
                    anchors.rightMargin: window.width > Units.dp(820) ? Units.dp(24) : Units.dp(16)

                    action: Action {
                        iconName: "av/skip_previous"
                        name: qsTr("Previous")
                        onTriggered: context.previous()
                    }

                    color: barControls.iconColor
                }
                IconButton {
                    id: bReplay

                    property bool showReplay: false;
                    readonly property bool buttonVisible: showReplay && context.playbackState !== MediaPlayer.StoppedState

                    opacity: buttonVisible ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 400 } }

                    //anchors.top: bPlay.top
                    //anchors.topMargin: bPlay.height/2 - height/2
                    anchors.verticalCenter: bPlay.verticalCenter
                    anchors.right: bPlay.left
                    anchors.rightMargin: buttonVisible ? (window.width > Units.dp(820) ? Units.dp(24) : Units.dp(16)) : -width
                    Behavior on anchors.rightMargin {
                        NumberAnimation { duration: 480; easing.type: Easing.InOutCubic }
                    }

                    action: Action {
                        iconName: "av/replay"
                        name: qsTr("Replay")
                        onTriggered: context.setMediaPosition(0)
                        shortcut: "Ctrl+R"
                    }

                    color: barControls.iconColor
                }
				IconButton {
					id: bNext

					//anchors.top: bPlay.top
					//anchors.topMargin: bPlay.height/2 - height/2
					anchors.verticalCenter: bPlay.verticalCenter
					anchors.left: bPlay.right
                    anchors.leftMargin: window.width > Units.dp(820) ? Units.dp(24) : Units.dp(16)

					action: Action {
						iconName: "av/skip_next"
                        name: qsTr("Next")
                        onTriggered: context.next()
					}

					color: barControls.iconColor
				}

				Item {
					//width: library.width
					height: Units.dp(56)

					anchors {
						left: bNext.right
						leftMargin: Units.dp(12)
						right: bOsd.left
						rightMargin: Units.dp(8)
						verticalCenter: bPlay.verticalCenter
					}
					
					Label {
						id: lMediaTitle
                        enabled: width >= bMediaInfo.width*3 ? 1 : 0
                        opacity: enabled ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 280 } }
                        color: videoBehind ? Theme.dark.subTextColor : Theme.light.textColor
						
						anchors {
							left: parent.left
							right: bRepeat.left
							rightMargin: Units.dp(16)
							verticalCenter: bRepeat.verticalCenter
							//top: parent.top
							//bottom: bPlaylist.top
						}
						
						text: context.playlist.mediaTitle
						style: "subheading" // "subheading"
						horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
					}

                    IconButton {
                        id: bMediaInfo
                        enabled: opacity == 1
                        opacity: lMediaTitle.enabled ? 0 : 1 // lMediaTitle.width < width*3 ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 240 } }
                        color: barControls.iconColor
                        size: Units.dp(28)
                        anchors {
                            right: bRepeat.left
                            rightMargin: Units.dp(12)
                            top: parent.top
                            bottom: parent.bottom
                        }
                        action: Action {
                            iconName: "action/info_outline"
                            name: qsTr("Info")
                            // onTriggered: playlist.open ? hidePlaylist() : showPlaylist()
                        }
                    }
					
					IconButton {
						id: bRepeat
                        color: context.loopMode === MediaPlayerContext.Sequential ? barControls.iconColor : Theme.accentColor
						size: Units.dp(28)
						anchors {
							right: bShuffle.left
							rightMargin: Units.dp(12)
							top: parent.top
							bottom: parent.bottom
						}
						action: Action {
                            iconName: context.loopMode === MediaPlayerContext.CurrentItemInLoop ? "av/repeat_one" : "av/repeat"
                            name: qsTr("Repeat")
                            onTriggered: {
                                if(context.loopMode === MediaPlayerContext.Sequential) {
                                    context.loopMode = MediaPlayerContext.Loop;
                                } else if(context.loopMode === MediaPlayerContext.Loop) {
                                    context.loopMode = MediaPlayerContext.CurrentItemInLoop;
                                } else {
                                    context.loopMode = MediaPlayerContext.Sequential;
                                }
                            }
						}
					}
					
					IconButton {
						id: bShuffle
                        color: barControls.iconColor //context.playlist.isRandom ? theme.accentColor : barControls.iconColor
						size: Units.dp(28)
						anchors {
							right: bPlaylist.left
							rightMargin: Units.dp(12)
							top: parent.top
							bottom: parent.bottom
						}
						action: Action {
							iconName: "av/shuffle"
                            name: qsTr("Shuffle")
                            onTriggered: context.playlist.randomize() //.toggleRandom()
						}
					}
					
					IconButton {
						id: bPlaylist
                        color: playlist.open ? Theme.accentColor : barControls.iconColor
						size: Units.dp(28)
						anchors {
							right: parent.right
							top: parent.top
							bottom: parent.bottom
						}
						action: Action {
							iconName: "av/queue_music"
                            name: playlist.open ? qsTr("Hide playlist") : qsTr("Show playlist")
                            onTriggered: {
                                if(playlist.open) {
                                    hidePlaylist();
                                    if(!videoBehind || osdOpen) {
                                        playlist.wasOpen = false;
                                        context.settings.playlistSidebarOpen = false;
                                    }
                                } else {
                                    showPlaylist()
                                    if(!videoBehind || osdOpen) {
                                        playlist.wasOpen = true;
                                        context.settings.playlistSidebarOpen = true;
                                    }
                                }
                            }
						}
                        Rectangle {
                            id: bPlaylistAttention

                            property int size: 0
                            width: size
                            height: size
                            anchors.centerIn: parent
                            radius: size/2
                            color: Theme.accentColor
                            opacity: 0

                            function openDropZone() {
                                if(dropZoneAnimation.running || opacity > 0.6) return;

                                noDropZoneAnimation.stop();
                                dropZoneAnimation.start();
                            }
                            function closeDropZone() {
                                dropZoneAnimation.stop();
                                noDropZoneAnimation.start();
                            }

                            function requestAttention() {
                                opacity = .7;
                                attentionAnimation.start();
//                                animateOpacity.start();
//                                animateSize.start();
                            }

                            ParallelAnimation {
                                id: dropZoneAnimation

                                NumberAnimation {
                                    target: bPlaylistAttention
                                    properties: "size"
                                    from: 0
                                    to: bPlaylist.size*1.5
                                    duration: 300
                                    easing.type: Easing.InOutCubic
                                }
                                NumberAnimation {
                                    target: bPlaylistAttention
                                    properties: "opacity"
                                    from: 0
                                    to: .7
                                    duration: 200
                                    easing.type: Easing.InOutCubic
                                }
                            }
                            NumberAnimation {
                                id: noDropZoneAnimation
                                target: bPlaylistAttention
                                properties: "opacity"
                                to: 0
                                duration: 300
                                easing.type: Easing.InOutCubic
                            }

                            SequentialAnimation {
                                id: attentionAnimation

                                NumberAnimation {
                                    target: bPlaylistAttention
                                    properties: "size"
                                    from: 0
                                    to: bPlaylist.size*1.5
                                    duration: 300
                                    easing.type: Easing.InOutCubic
                                }
                                NumberAnimation {
                                    target: bPlaylistAttention
                                    properties: "opacity"
                                    from: .7
                                    to: 0
                                    duration: 500
                                    easing.type: Easing.InOutCubic
                                }
                            }

//                            NumberAnimation {
//                                id: animateOpacity
//                                target: bPlaylistAttention
//                                properties: "opacity"
//                                from: .5
//                                to: 0
//                                duration: 400
//                                easing.type: Easing.InBack //InOutCubic
//                                easing.overshoot: 10
//                                //easing {type: Easing.OutBack; overshoot: 500}
//                            }
//                            NumberAnimation {
//                                id: animateSize
//                                target: bPlaylistAttention
//                                properties: "size"
//                                from: bPlaylist.size
//                                to: 0
//                                duration: 400
//                                easing.type: Easing.InBack //InOutCubic
//                                easing.overshoot: Units.dp(8)
//                                //easing {type: Easing.OutBack; overshoot: 500}
//                            }
                        }
					}
				}
				
				IconButton {
					id: bOsd
					color: barControls.iconColor
					enabled: videoBehind
					
					size: Units.dp(52)
					
					anchors {
						right: parent.right
						rightMargin: videoBehind ? Units.dp(24) : -width + Units.dp(24)
						verticalCenter: bPlay.verticalCenter
					}
					
					opacity: videoBehind ? 1 : 0
					
					Behavior on anchors.rightMargin {
						NumberAnimation {
							duration: 320
							easing.type: Easing.InOutQuad
						}
					}
					Behavior on opacity {
						NumberAnimation {
							duration: 280
							easing.type: Easing.InOutQuad
						}
					}
					
					action: Action {
						iconName: osdOpen ? "navigation/fullscreen_exit" : "navigation/fullscreen" // "action/settings_overscan"
                        name: osdOpen ? qsTr("Back to the video") : qsTr("Show navigator")
						onTriggered: osdOpen ? closeOsd() : openOsd()
					}
				}
			}

            SeekBar {
                id: mediaProgress
                objectName: "mediaProgress"

                property int setValueWhenPossible: -1;
                onMaximumValueChanged: {
                    if(setValueWhenPossible != -1) {
                        value = setValueWhenPossible;
                        setValueWhenPossible = -1;
                    }
                }

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right

                value: 0

                seekable: context.isSeekable
                onSeeked: context.setMediaPosition(position)

                revertPositionShortcut: shortcutRevertPosition.nativeText
            }
        }

        Snackbar {
            id: snackbar
            z: 100

            property var clickCallback;

            function show(text, time, button, onClicked) {
                if(button && onClicked) {
                    clickCallback = onClicked;
                    buttonText = button;
                }
                duration = time;
                open(text);
            }

            onClicked: {
                opened = false;
                clickCallback();
            }
        }
    }

    function hideLibrary() {
        //library.anchors.leftMargin = -library.width
        library.open = false;
    }

    function hidePlaylist() {
        //playlist.anchors.rightMargin = -playlist.width
        playlist.open = false;
    }

    function showLibrary() {
        //library.anchors.leftMargin = 0
        library.open = true;
    }

    function showPlaylist() {
        //playlist.anchors.rightMargin = 0
		playlist.open = true;
    }

    function openOsd() {
        userOpenedOsd = true;
		osdOpen = true;
		if(library.wasOpen) showLibrary();
		if(playlist.wasOpen) showPlaylist();
        cursorVisible = true;
    }
    
    function closeOsd() {
        userOpenedOsd = false;
		osdOpen = false;
		if(library.open) hideLibrary();
		if(playlist.open) hidePlaylist();
    }

    function toggleFullscreen() {
		visibility = visibility == Window.FullScreen ? Window.Windowed : Window.FullScreen;
    }

    function askUserToOpenMedia() {
        if(false && mediaOpenerDialog) {
            mediaOpenerDialog.open();
        } else {
            var component = Qt.createComponent("MediaOpenerDialog.qml")
            var obj = component.createObject(root)
            obj.onAccepted.connect(function(){onMediaOpened(mediaOpenerDialog.fileUrls); /*mediaOpenerDialog.fileUrls.length = 0;*/})
            mediaOpenerDialog = obj // BUG: QT's file dialog doesn't clear "fileUrls" -.-
            obj.open()
        }
    }

    function openSettings(page) {
        var component = Qt.createComponent("SettingsWindow.qml");
        var window = component.createObject(window);
        if(window) {
            //component.theme = theme;
            if(page) window.openPage(page);
            window.x = x + width/2 - window.width/2;
            window.y = y + height/2 - window.height/2;
            window.show();
        } else {
            console.log(component.errorString());
        }
    }

    function showMediaPropertyRemovalSnackbar(name) {
        snackbar.show(qsTr("%1 removed from your library.").arg(name), 10000, qsTr("Undo", "Shown after the user removes an artist/tv show/etc. from their library."), libraryModel.undoRemoval);
    }

    Shortcut {
        sequence: "ESC"
        onActivated: {
            if(tSearchBox.activeFocus) {
                navigator.forceActiveFocus();
                return;
            }
            if(videoBehind) {
                if(osdOpen) { closeOsd(); }
                else {
                    if(visibility == Window.FullScreen) toggleFullscreen();
                    else openOsd();
                }
            } else if(visibility == Window.FullScreen) toggleFullscreen();
        }
    }

    Shortcut {
        id: shortcutRevertPosition
        sequence: "Ctrl+Z" // "Ctrl+Backspace"
        onActivated: mediaProgress.revertPosition()
        autoRepeat: false
    }

    Shortcut {
        sequence: "Ctrl+Backspace"
        enabled: navigator.depth > 1
        onActivated: if(!videoBehind && !osdOpen) navigator.pop();
        autoRepeat: false
    }

//    Shortcut {
//        sequence: Qt.Key_MediaPlay
//        onActivated: bPlay.action.trigger(bPlay)
//    }
//    Shortcut {
//        sequence: Qt.Key_MediaPrevious
//        onActivated: bPrevious.action.trigger(bPrevious)
//    }
//    Shortcut {
//        sequence: Qt.Key_MediaNext
//        onActivated: bNext.action.trigger(bNext)
//    }

//    Keys.onLeftPressed: _context.mediaPosition -= 2000;
//    Keys.onRightPressed: _context.mediaPosition += 2000;

	Shortcut {
        sequence: "Ctrl+M"
        onActivated: _context.playbackRate = (_context.playbackRate === -1 ? 1 : -1)
        autoRepeat: false
    }

    Shortcut {
        id: shortcutFind
        sequence: "Ctrl+F"
        onActivated: {
            tSearchBox.forceActiveFocus();
            tSearchBox.selectAll();
        }
        autoRepeat: false
    }

	Shortcut {
        sequence: "Ctrl+8"
        onActivated: _context.playbackRate -= 0.05
    }
	Shortcut {
        sequence: "Ctrl+9"
        onActivated: _context.playbackRate += 0.05
    }
	Shortcut {
        sequence: "Ctrl+7"
        onActivated: _context.playbackRate /= 2
    }
	Shortcut {
        sequence: "Ctrl+0"
        onActivated: _context.playbackRate *= 2
    }
}
