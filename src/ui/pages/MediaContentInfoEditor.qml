import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Material 0.2
//import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

Dropdown {
    id: root

    width: Units.dp(320)
    height: wave.opened ? extraInfoContents.height : contents.height // Units.dp(400)
    Behavior on height {
        NumberAnimation { duration: 280; easing.type: Easing.InOutCubic }
    }

    internalView.elevation: 6

    property bool editionMode: false

//    Card {
//        id: root

//        elevation: 4

//        width: parent.width
//        anchors.centerIn: parent
////        anchors {
////            top: parent.top
////            topMargin: Units.gu(1)
////            left: parent.left
////            right: parent.right
////            rightMargin: root.rightMargin
////        }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    ColumnLayout {
        id: contents

        clip: true

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            id: toolbar

            height: Units.gu(0.7)
            Layout.fillWidth: true
//            anchors {
//                left: parent.left
//                top: parent.top
//                right: parent.right
//            }
            clip: true
            color: Theme.light.accentColor
            radius: Units.dp(1)

            Rectangle {
                // just to cover the bottom corners
                height: parent.height / 2
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    right: parent.right
                }
                color: parent.color
            }

            IconButton {
                id: bClose

                width: Units.dp(40)
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                action: Action {
                    iconName: "navigation/close"
                    text: qsTr("Close")
                    onTriggered: if(editionMode) editionMode = false; else root.close();
                }

                color: Theme.dark.iconColor
            }

            Label {
                anchors.left: bClose.right
                anchors.leftMargin: Units.dp(4)
                anchors.verticalCenter: parent.verticalCenter

                text: editionMode ? qsTr("Edit") : qsTr("Info")
                style: "title"

                color: Theme.dark.textColor
            }

            Button {
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    bottomMargin: editionMode ? Units.dp(8) : 0
                }

                action: Action {
                    iconName: "content/create"
                    text: qsTr("Make changes")
                    onTriggered: editionMode = true;
                }

                textColor: Theme.dark.textColor
                opacity: editionMode ? 0 : 1
                enabled: opacity == 1
                z: opacity == 1 ? 1 : 0

                Behavior on anchors.bottomMargin { NumberAnimation { duration: 200; easing.type: Easing.InOutCubic } }
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }

            Button {
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    bottomMargin: editionMode ? 0 : -Units.dp(8)
                }

                action: Action {
                    iconName: "content/create"
                    text: qsTr("Save")
                    //onTriggered: editionMode = true;
                }

                textColor: Theme.dark.textColor
                opacity: editionMode ? 1 : 0
                enabled: opacity == 1
                z: opacity == 1 ? 1 : 0

                Behavior on anchors.bottomMargin { NumberAnimation { duration: 200; easing.type: Easing.InOutCubic } }
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }
        }

        ColumnLayout {
            Layout.leftMargin: Units.dp(8)
            Layout.rightMargin: Units.dp(8)
//            anchors {
//                top: toolbar.bottom
//                topMargin: Units.dp(8)
//                left: parent.left
//                leftMargin: Units.dp(8)
//                right: parent.right
//                rightMargin: Units.dp(8)
//                bottom: parent.bottom
//            }

            TextField {
                id: mediaName

                Layout.fillWidth: true
                Layout.topMargin: -Units.dp(3)
//                anchors.top: parent.top
//                anchors.left: parent.left

                text: "Media name"
                font.pixelSize: Units.dp(19)
                showBorder: editionMode

                onFocusChanged: if(!editionMode) editionMode = true;
            }

            Ink {
                id: bType

                width: Math.max(Units.dp(240), root.width/2)
                height: Units.dp(32)

                Layout.fillWidth: true
                Layout.leftMargin: -Units.dp(8)
                Layout.rightMargin: -Units.dp(8)
                Layout.topMargin: editionMode ? Units.dp(-8) : Units.dp(-14)
                Layout.bottomMargin: Units.dp(-4)

                Behavior on Layout.topMargin { NumberAnimation { duration: 220; easing.type: Easing.InOutCubic } }
//                anchors {
//                    left: mediaName.left
//                    top: mediaName.bottom
//                    topMargin: Units.dp(4)
//                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: Units.dp(8)
                    anchors.rightMargin: Units.dp(8)
                    spacing: Units.dp(1)
                    Label {
                        id: mediaType

                        text: "Media type"
                        //anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        style: "subheading"
                    }

                    Icon {
                        name: "navigation/arrow_drop_down"
                        anchors.verticalCenter: parent.verticalCenter
                        //anchors.left: mediaType.right
                        //anchors.leftMargin: Units.dp(4)
                        opacity: editionMode ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: 180 } }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: -Units.dp(8)
                Layout.rightMargin: -Units.dp(8)
                height: 1
                color: Theme.light.dividerColor
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: -Units.dp(8)
                Layout.rightMargin: -Units.dp(8)
                Layout.bottomMargin: Units.dp(6)
                height: 1
                color: "#ffffff"
            }

            Row {
                id: rArtist

                Layout.fillWidth: true
                Layout.bottomMargin: editionMode && tArtist.helperText.length > 0 ? Units.dp(34) : Units.dp(10)
                Behavior on Layout.bottomMargin { NumberAnimation { duration: 220; easing.type: Easing.InOutCubic } }
//                anchors {
//                    left: parent.left
//                    right: parent.right
//                    top: bType.bottom
//                    topMargin: Units.dp(24)
//                }

                TextField {
                    id: tArtist

                    width: rArtist.width //Math.max(Units.dp(160), root.width/2)

                    floatingLabel: true
                    placeholderText: qsTr("Artist")
                    text: "Test"
                    helperText: text.length > 0 ? text + " will be added to the library" : ""
                    showBorder: editionMode
                }
            }

            Row {
                id: rAlbum

                Layout.fillWidth: true
                Layout.bottomMargin: Units.dp(2)
//                anchors {
//                    left: parent.left
//                    right: parent.right
//                    top: rArtist.bottom
//                    topMargin: tArtist.helperText.length > 0 ? Units.dp(38) : Units.dp(20)
//                }
                spacing: Units.dp(8)

                TextField {
                    id: tAlbum

                    width: Math.max(Units.dp(120), 2*(rAlbum.width-parent.spacing)/3)

                    floatingLabel: true
                    placeholderText: qsTr("Album")
                    text: "Test"
                    showBorder: editionMode
                }

                TextField {
                    id: tTrackNumber

                    width: Math.max(Units.dp(80), (rAlbum.width-parent.spacing)/3)

                    floatingLabel: true
                    placeholderText: qsTr("Track number")
                    text: "1"
                    showBorder: editionMode

                    validator: IntValidator { bottom: 0 }
                }
            }

            Label {
                id: lFeat

                Layout.bottomMargin: -Units.dp(4)

//                anchors {
//                    left: parent.left
//                    right: parent.right
//                    top: rAlbum.bottom
//                    topMargin: tAlbum.helperText.length > 0 ? Units.dp(16) : Units.dp(8)
//                }

                text: qsTr("Featuring")
                color: Theme.light.subTextColor
                opacity: 0.72
                style: "caption"
            }

            Column {
                Layout.fillWidth: true
//                anchors {
//                    left: parent.left
//                    right: parent.right
//                    top: lFeat.bottom
//                    topMargin: Units.dp(8)
//                }

                Repeater {
                    model: 3

                    delegate: Item {
                        height: Units.dp(32)
                        anchors.left: parent.left
                        anchors.right: parent.right

                        TextField {
                            anchors {
                                left: parent.left
                                right: bClear.left
                                rightMargin: Units.dp(12)
                                verticalCenter: parent.verticalCenter
                            }

                            text: "Hello"
                            showBorder: editionMode
                        }
                        IconButton {
                            id: bClear
                            anchors {
                                right: parent.right
                                rightMargin: Units.dp(8)
                                top: parent.top
                                bottom: parent.bottom
                            }

                            iconName: "content/clear"
                        }
                    }
                }

                Item {
                    height: Units.dp(32)
                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        anchors {
                            left: parent.left
                            right: bAdd.left
                            rightMargin: Units.dp(12)
                            verticalCenter: parent.verticalCenter
                        }

                        placeholderText: qsTr("Add")
                        showBorder: editionMode
                    }
                    IconButton {
                        id: bAdd
                        anchors {
                            right: parent.right
                            rightMargin: Units.dp(8)
                            top: parent.top
                            bottom: parent.bottom
                        }

                        iconName: "content/add"
                    }
                }
            }
        }

        Button {
            id: bTI

            Layout.fillWidth: true
            Layout.leftMargin: Units.dp(8)
            Layout.rightMargin: Units.dp(8)
            Layout.topMargin: Units.dp(8)
            Layout.bottomMargin: editionMode ? Units.dp(-6) : Units.dp(8)
            text: qsTr("Technical info");
            backgroundColor: Palette.colors['blueGrey'][800]
            elevation: 1
            opacity: editionMode ? 0 : 1
            //scale: editionMode ? 0.7 : 1
            //visible: opacity != 0
            //implicitHeight: opacity == 0 ? 0 : height
            implicitHeight: editionMode ? 0 : Units.dp(36)
            //implicitHeight: height
            height: implicitHeight

            Behavior on opacity { NumberAnimation { duration: 360 } }
            //Behavior on scale { NumberAnimation { duration: 380; easing.type: Easing.InOutCubic } }
            Behavior on Layout.bottomMargin { NumberAnimation { duration: 380; easing.type: Easing.InOutCubic } }
            Behavior on implicitHeight { NumberAnimation { duration: 480; easing.type: Easing.InOutCubic } }


            onClicked: {
                //var wave = Qt.createQmlObject("import Material 0.2; Wave {}", contents);
                //wave.color = bTI.backgroundColor;
                // TODO: should start where the mouse is...
                wave.open(bTI.width / 2 + bTI.x, bTI.height / 2 + bTI.y);
            }
        }
    }

    Wave {
        id: wave
        color: bTI.backgroundColor
    }

    ColumnLayout {
        id: extraInfoContents

        clip: true

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        visible: opacity != 0
        opacity: wave.opened ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 180 } }

        Rectangle {
            id: toolbarTI

            height: Units.gu(0.7)
            Layout.fillWidth: true
    //            anchors {
    //                left: parent.left
    //                top: parent.top
    //                right: parent.right
    //            }
            clip: true
            color: bTI.backgroundColor
            radius: Units.dp(1)

            IconButton {
                id: bBack

                width: Units.dp(40)
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                action: Action {
                    iconName: "navigation/arrow_back"
                    text: qsTr("Back")
                    onTriggered: wave.close(bBack.width / 2 + bBack.x, bBack.height / 2 + bBack.y);
                }

                color: Theme.dark.iconColor
            }

            Label {
                anchors.left: bBack.right
                anchors.leftMargin: Units.dp(4)
                anchors.verticalCenter: parent.verticalCenter

                text: qsTr("Technical info")
                style: "title"

                color: Theme.dark.textColor
            }
        }

        Label {
            text: qsTr("Location")
            Layout.leftMargin: Units.dp(8)
            Layout.rightMargin: Units.dp(8)
            Layout.bottomMargin: -Units.dp(10)
            style: "caption"
            color: Theme.dark.subTextColor
        }
        Item {
            height: Math.max(lFolder.height, bFolder.height)
            Layout.leftMargin: Units.dp(8)
            Layout.rightMargin: Units.dp(8)
            Layout.fillWidth: true
            Label {
                id: lFolder
                anchors {
                    left: parent.left
                    right: bFolder.right
                    verticalCenter: parent.verticalCenter
                }
                text: "/home/.../.../....mp3"
                style: "body1"
                color: Theme.dark.textColor
            }
            IconButton {
                id: bFolder
                height: Units.dp(40)
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                action: Action {
                    text: qsTr("Open folder")
                    iconName: "file/folder"
                }
                color: Theme.dark.iconColor
            }
        }
        Label {
            Layout.leftMargin: Units.dp(8)
            Layout.rightMargin: Units.dp(8)
            Layout.bottomMargin: Units.dp(8)
            text: "// TODO"
            style: "body1"
            color: Theme.dark.textColor
        }
    }
//    }
}
