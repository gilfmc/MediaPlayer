import QtQuick 2.5

import Material 0.2
import Material.ListItems 0.1 as ListItems

import "components"

Page {
    title: qsTr("Library")

    property var lookupPlaces

    actions: [
        Action {
            text: qsTr("Revert changes")
            iconName: "action/settings_backup_restore"
            onTriggered: {
                undoToast.opened = false;
                loadPlaces();
            }
            visible: lookupPlaces.hasChangesToSave
        },
        Action {
            text: qsTr("Delete library")
            iconName: "action/delete"
            onTriggered: {
                if(canDeleteLibrary()) {
                    dialog.open();
                }
            }
        },
        Action {
            text: qsTr("Help")
            iconName: "action/help"
        }
    ]
    actionBar.maxActionCount: 2

    function canDeleteLibrary() {
        if(context.isLibraryBeingRefreshed()) {
            uiState.showSnackbar(qsTr("The library is being refreshed right now, please try once it's done."));
            return false;
        }
        return true;
    }

    Loader {
        id: dialog

        function open() {
            sourceComponent = componentDialog;
            item.open();
        }

        Component {
            id: componentDialog
            Dialog {
                title: qsTr("Delete library")
                dialogContent: Item {
                    width: Math.max(parent.width, children[1].width)
                    height: children[0].height + children[1].height - Units.dp(16)
                    Label {
                        id: dialogText
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: qsTr("All your songs, episodes, etc. will be removed from your library. You can keep your list of artists, TV shows, etc. The app may automatically add your songs and episodes if it finds them again.\nDo you really want to delete your library?")
                        wrapMode: Text.Wrap
                        style: "subheading"
                    }
                    Button {
                        context: "dialog"
                        anchors {
                            top: dialogText.bottom
                            topMargin: Units.dp(8)
                            right: parent.right
                            rightMargin: -Units.dp(16)
                        }

                        text: qsTr("Keep artists, TV shows, etc.")
                        textColor: Theme.accentColor
                        onClicked: {
                            if(canDeleteLibrary()) {
                                _context.library.deleteLibrary(true, this);
                                dialog.item.close();
                            }
                        }
                    }
                }

                positiveButtonText: qsTr("Delete everything")
                negativeButtonText: qsTr("Cancel")

                onAccepted: if(canDeleteLibrary()) _context.library.deleteLibrary(false, this);
            }
        }
    }

    Label {
        id: lPlaces

        text: qsTr("Look for music and videos in")
        style: "body2"

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Units.dp(16)
    }

    Rectangle {
        anchors.bottom: lPlaces.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: 1

        color: Qt.rgba(0,0,0,0.1)

        opacity: !placesList.atYBeginning

        Behavior on opacity { NumberAnimation { duration: 180 } }
    }

    Scrollbar {
        flickableItem: placesList
    }

    ListView {
        id: placesList

        property var openItem : undefined

        clip: true

        anchors {
            top: lPlaces.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
//        footer: ListItems.Standard {
//            text: qsTr("Add folder")
//            iconName: "content/add"
//            onClicked: {
//                var folderDialog = Qt.createQmlObject('import QtQuick 2.0; import QtQuick.Dialogs 1.2; FileDialog { selectFolder: true}', parent);
//                folderDialog.onAccepted.connect(function() {
//                    addPlace(lookupPlaces.add(folderDialog.folder));
//                });
//                folderDialog.open();
//            }
//        }
        bottomMargin: bAdd.height + bAdd.anchors.bottomMargin*2

        property Item configurationPanel: Column {
            id: configurationPanel

            property alias path: lPath.text
            property alias includeHidden: cbHidden.checked
            property alias includeRecursive: cbRecursive.checked

            anchors.fill: parent
            anchors.leftMargin: -Units.dp(8)
            anchors.rightMargin: -Units.dp(8)
            anchors.topMargin: Units.dp(8)
            anchors.bottomMargin: Units.dp(8)

            Row {
                width: parent.width + Units.dp(32)
                height: Math.max(lPath.implicitHeight + Units.dp(32), bChange.implicitHeight)
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: Units.dp(8)
                anchors.rightMargin: Units.dp(8)

                TextField {
                    id: lPath
                    width: parent.width - bChange.width
                    helperText: qsTr("Place")
                    floatingLabel: true
                    //anchors.verticalCenter: parent.verticalCenter
                }

                Button {
                    id: bChange
                    text: qsTr("Change")
                    anchors.verticalCenter: lPath.verticalCenter
                }
            }

            CheckBox {
                id: cbHidden
                height: implicitHeight - Units.dp(12)
                text: qsTr("Include hidden folders and files")
            }
            CheckBox {
                id: cbRecursive
                height: implicitHeight - Units.dp(12)
                text: qsTr("Look for files in subfolders")
            }

            Row {
                Button {
                    text: qsTr("Cancel")
                }
                Button {
                    text: qsTr("Save")
                }
            }
        }

        model:
             ListModel {
                id: lookupPlacesModel
             }
        delegate: ListItems.Standard {
            id: item

//            interactive: type != 0
//            height: Units.dp(window.height > 12*Units.gridUnit ? (type == 0 ? 64 : 72) : (type == 0 ? 56 : 64))
            text: name
            secondaryItem: [
                Row {
                    height: Units.dp(48)

                    anchors {
                        top: parent.top
                        right: parent.right
                    }

                    spacing: Units.dp(16)

                    IconButton {
                        id: bConfigure
                        action: Action {
                            iconName: "action/settings"
                            text: qsTr("Configure")
                        }
                        anchors {
                            verticalCenter: parent.verticalCenter
                        }
                        onClicked: toggle()
                    }
                    IconButton {
                        id: bRemove
                        action: Action {
                            iconName: "content/clear"
                            text: qsTr("Remove")
                        }
                        anchors {
                            verticalCenter: parent.verticalCenter
                        }
                        onClicked: {
                            bRemove.enabled = false;
                            undoToast.open(qsTr("%1 removed.", "Shown when the user deletes items in the library settings page.").arg(name));
                            lookupPlaces.remove(index);
                            lookupPlacesModel.remove(index);
                        }
                    }
                }
            ]
            onClicked: {
                toggle();
            }

            content: elevation == 0 ? [] : placesList.configurationPanel
            height: elevation == 0 ? implicitHeight : (placesList.configurationPanel.implicitHeight + margins)

            Behavior on elevation {
                NumberAnimation { duration: 120 }
            }

            Behavior on height {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }

            function toggle() {
                if(elevation == 0) {
                   open();
                } else {
                   close();
                }
            }

            function open() {
                if(path) {
                    configurationPanel.path = path

                    if(parent.openItem && parent.openItem.close) parent.openItem.close();
                    parent.openItem = item;

                    elevation = 2;
                    item.backgroundColor = "#ffffff";
                    radius = Units.dp(2);
                }
            }

            function close() {
                elevation = 0;
                item.backgroundColor = "#00ffffff";
                radius = 0;

                if(parent.openItem == item) parent.openItem = undefined;
            }

//            subText: description
//            iconName: interactive ? "action/extension" : ""
        }

        add: Transition {
            ParallelAnimation {
                NumberAnimation { properties: "opacity"; from: 0; to: 1; duration: 180 }
                NumberAnimation { properties: "anchors.leftMargin"; from: Units.dp(64); to: 0; duration: 240; easing.type: Easing.OutCubic }
            }
        }

        remove: Transition {
            ParallelAnimation {
                NumberAnimation { properties: "opacity"; from: 1; to: 0; duration: 200 }
                NumberAnimation { properties: "anchors.leftMargin"; from: 0; to: Units.dp(48); duration: 260; easing.type: Easing.InOutCubic }
            }
        }

        displaced: Transition {
            SequentialAnimation {
                PauseAnimation { duration: 80 }
                NumberAnimation { properties: "y"; duration: 200; easing.type: Easing.InOutCubic }
            }
        }
    }

    FloatingActionButton {
        id: bAdd

        action: Action {
            iconName: "content/add"
            name: qsTr("Add folder to look at")
            onTriggered: {
                var folderDialog = Qt.createQmlObject('import QtQuick 2.0; import QtQuick.Dialogs 1.2; FileDialog { selectFolder: true}', parent);
                folderDialog.onAccepted.connect(function() {
                    addPlace(lookupPlaces.add(folderDialog.folder));
                    placesList.currentIndex = placesList.count - 1;
                });
                folderDialog.open();
            }
        }

        snackbar: undoToast
    }

    Component.onCompleted: {
        loadPlaces();
    }

    Snackbar {
        id: undoToast

        // TODO:
//        buttonText: qsTr("Undo")
//        onClicked: {

//        }
    }

    function loadPlaces() {
        lookupPlacesModel.clear();
        lookupPlaces = context.settings.library.lookupPlaces;
        for(var i = 0; i < lookupPlaces.length; i++) {
            addPlace(lookupPlaces.at(i));
        }
        //lookupPlacesModel.append({name: "Home folder", path: "/home/gil"});
        //lookupPlacesModel.list = places;
        //lookupPlaces = places;
    }

    function addPlace(place) {
        lookupPlacesModel.append({name: lookupPlaces.getUserReadablePlaceName(place), path: place.path, recursive: place.recursive, includeHidden: place.includeHidden});
    }

    function save() {
        lookupPlaces.save();
    }
}
