import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Material 0.2
//import Material.Extras 0.1 as Extras
import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

NavigatorPage {
    id: root

    property PropertyListUiProvider ui;

    property int itemSize: Units.dp(120)
    property int cellWidth: ui ? (itemSize + Units.dp(root.height > Units.dp(600) ? (ui.viewType == 1 ? 88 : 72) : (ui.viewType == 1 ? 48 : 40))) : 1

    property bool largeItems: grid.cellWidth > Units.dp(160);

    GridView {
        id: grid

        topMargin: topBar.height / 1.4
        bottomMargin: uiState.videoBehind && uiState.osdOpen ? uiState.bottomBarHeight : 0

//        anchors.fill: parent

        anchors {
            top: parent.top
            left: parent.left
            //leftMargin: Units.dp(16)
            right: parent.right
            rightMargin: root.rightMargin
            topMargin: /*Units.dp(16) +*/ toolbarMain.height
            bottom: parent.bottom
        }

        model: ui

        //Component.onCompleted: model = 100;

        property int columnCount: Math.max(1, Math.round(width / root.cellWidth))
        cellWidth: width / columnCount
        cellHeight: ui ? (ui.viewType == 1 ? Units.dp(root.cellWidth + Units.dp(8)) : cellWidth*1.48) : 1

        populate: Transition {
            id: lpaa
            SequentialAnimation {
                NumberAnimation { property: "opacity"; from: 0; to: 0; duration: 0 }
                PauseAnimation {
                    property int index: (lpaa.ViewTransition.index - lpaa.ViewTransition.targetIndexes[0]);
                    property int column: index % grid.columnCount;
                    property int row: index / grid.columnCount
                    duration: (column + row) * 45 + 60
                }
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; easing.type: Easing.InOutCubic; duration: 360 }
                    NumberAnimation { property: "scale"; from: 0.8; to: 1; easing.type: Easing.InOutCubic; duration: 380 }
                }
            }
        }
        add: populate
        remove: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; easing.type: Easing.InOutCubic; duration: 380 }
            NumberAnimation { property: "scale"; from: 1; to: 0.8; easing.type: Easing.InOutCubic; duration: 400 }
        }

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 280; easing.type: Easing.InOutCubic }
        }

        delegate: ui ? (ui.viewType == 1 ? delegatePerson : delegateCard) : null
    }

    View {
        id: topBar

        width: grid.width
        height: Units.dp(48)

        anchors {
            top: parent.top
            left: parent.left
            //leftMargin: Units.dp(16)
            right: parent.right
            rightMargin: root.rightMargin
            topMargin: /*Units.dp(16) +*/ toolbarMain.height
        }

        elevation: !grid.atYBeginning || (uiState && uiState.videoBehind && uiState.osdOpen) ? 1 : 0

        Label {
            text: ui ? ui.readableCount : ""
            style: "subheading"
            anchors.left: parent.left
            anchors.leftMargin: Units.dp(16)
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Button {
                anchors.top: parent.top
                anchors.bottom: parent.bottom;
                text: qsTr("Add")
                onClicked: uiState.openAddMediaPropertyDialog(qsTr("Add"), ui.groupName, false)
            }
            IconButton {
                id: buttonMore
                anchors.top: parent.top
                anchors.bottom: parent.bottom;
                width: Units.dp(48)
                action: Action {
                    iconName: "navigation/more_vert"
                    name: qsTr("More options")
                    onTriggered: {
                        menuMore.open(buttonMore);
                    }
                }

                Loader {
                    id: menuMore

                    function open(from) {
                        if(menu) {
                            sourceComponent = menu;
                            menu = null;
                        }
                        item.open(from);
                    }

                    property Component menu: Component {
                        PopupMenu {
                            actions: [
                                Action {
                                    name: qsTr("Show all")
                                    checkable: true
                                    checked: ui.showAll

                                    onCheckedChanged: {
                                        ui.showAll = !ui.showAll
                                    }
                                }
                            ]
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: contextMenu

        function open(fromItem, id, type, inLibrary) {
            if(menu) {
                sourceComponent = menu;
                menu = null;
            }
            item.id = id;
            item.type = type;
            item.inLibrary = inLibrary === false ? false : true;
            item.openMenu(fromItem);
        }

        property Component menu: Component { PopupMenu {
            anchor: Item.TopLeft

            property int id
            property string type
            property bool inLibrary

            function openMenu(fromItem) {
                _actionInLibrary.checked = inLibrary;
                open(fromItem);
            }

            actions: [
                Action {
                    iconName: "av/play_arrow"
                    name: qsTr("Play")
                    onTriggered: uiState.playAll(type, id);
                }, Action {
                    iconName: "av/play_arrow"
                    name: qsTr("Play after this one")
                    onTriggered: uiState.showSnackbar(qsTr("This option will be available in an future version."), 2000)
                }, Action {
                    iconName: "content/create"
                    name: qsTr("Rename")
                    onTriggered: uiState.openRenameMediaPropertyDialog(type, id);
                }, Action {
                    id: _actionInLibrary
                    name: qsTr("In library")
                    checkable: true
                    checked: inLibrary
                    onTriggered: {
                        if(checked) {
                            uiState.addToLibrary(type, id);
                        } else {
                            uiState.removeFromLibrary(type, id);
                        }
                    }
                }, Action {
                    iconName: "action/delete"
                    name: qsTr("Remove permanently")
                    visible: !inLibrary
                    onTriggered: uiState.showSnackbar(qsTr("This option will be available in an future version."), 2000)
                }

            ]
        }}
    }

    Component {
        id: delegateCard
        Item {
            width: grid.cellWidth
            height: grid.cellHeight

            Card {
                anchors {
                    fill: parent
                    margins: Units.dp(8)
                }

                Image {
                    id: image

                    //width: itemSize
                    //height: itemSize

                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                        rightMargin: -1
                        bottom: parent.bottom
                        bottomMargin: -1
                    }

                    source: imageUrl
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop

                    sourceSize.width: Units.dp(192)
                }
                LinearGradient {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right

                    height: label.height * 3

                    cached: true

                    start: Qt.point(0, height)
                    end: Qt.point(0, 0)

                    gradient: Gradient {
                        GradientStop { position: 0.00; color: "#66000000" } // #33
                        GradientStop { position: 0.33; color: "#32000000" } // #19
                        GradientStop { position: 1.00; color: "#00000000" }
                    }
                }
                LinearGradient {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    height: menuButton.height * 3

                    cached: true

                    start: Qt.point(0, 0)
                    end: Qt.point(0, height)

                    gradient: Gradient {
                        GradientStop { position: 0.00; color: "#66000000" } // #33
                        GradientStop { position: 0.33; color: "#32000000" } // #19
                        GradientStop { position: 1.00; color: "#00000000" }
                    }
                }
                Label {
                    id: label

                    //height: Units.dp(32)

                    //anchors.top: image.bottom
                    //anchors.topMargin: Units.dp(4)
                    anchors.left: parent.left
                    anchors.leftMargin: largeItems ? Units.dp(16) : Units.dp(8)
                    anchors.right: parent.right
                    anchors.rightMargin: largeItems ? Units.dp(16) : Units.dp(8)
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: largeItems ? Units.dp(18) : Units.dp(9)

                    elide: Text.ElideRight
                    style: largeItems ? "title" : "subheading"
                    font.weight: Font.DemiBold
                    //verticalAlignment: Text.AlignVCenter
                    //horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    lineHeight: 0.8
                    color: Theme.dark.textColor

                    text: title
                }
                Ink {
                    id: ink
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        if(mouse.button == Qt.LeftButton)
                            openPage("MediaContentPage").ui = ui.getItemUi(i);
                        else if(mouse.button == Qt.RightButton)
                            contextMenu.open(label, id, ui.groupName, inLibrary);
                    }
                    anchors.fill: parent
                    color: Qt.rgba(1,1,1,0.1)

                    z: 1
                }
                IconButton {
                    id: menuButton

                    anchors.right: parent.right
                    anchors.rightMargin: largeItems ? Units.dp(8) : Units.dp(4)
                    anchors.top: parent.top
                    anchors.topMargin: largeItems ? Units.dp(8) : Units.dp(4)

                    size: largeItems ? Units.dp(20) : Units.dp(16)

                    iconName: "navigation/more_vert"
                    color: Theme.dark.iconColor
                    opacity: 0.7

                    z: 2

                    onClicked: contextMenu.open(menuButton, id, ui.groupName, inLibrary)
                }
            }
        }
    }

    Component {
        id: delegatePerson
        Item {
            width: grid.cellWidth
            height: grid.cellHeight

            Ink {
                id: ink
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if(mouse.button == Qt.LeftButton)
                        openPage("MediaContentPage").ui = ui.getItemUi(i);
                    else if(mouse.button == Qt.RightButton)
                        contextMenu.open(label, id, ui.groupName, inLibrary);
                }
                anchors.fill: parent
            }

            CircleImage {
                id: image

                width: itemSize
                height: itemSize

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: parent.height/2 - height/2 - label.height/label.lineCount/2
                }

                elevation: 1

                source: imageUrl
                asynchronous: true
                fillMode: Image.PreserveAspectCrop

                sourceSize.width: Units.dp(208) // grid.cellHeight
                sourceSize.height: Units.dp(208) // grid.cellHeight
            }
            Label {
                id: label

                anchors.top: image.bottom
                anchors.topMargin: Units.dp(4)
                anchors.left: parent.left
                anchors.right: parent.right

                elide: Text.ElideRight
                style: "subheading"
                horizontalAlignment: Text.Center
                wrapMode: Text.Wrap
                maximumLineCount: 2
                color: Theme.light.textColor

                text: title
            }
        }
    }

    Component.onCompleted: {
        onHeightChanged.connect(function() {
            itemSize = Units.dp(height > Units.dp(500) ? 120 : 80)
        });
    }
}
