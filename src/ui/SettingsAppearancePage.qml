import QtQuick 2.5
import QtQuick.Controls 1.4

import Material 0.2
import Material.ListItems 0.1 as ListItems

import "components"

Page {
    title: qsTr("Appearance")

    tabs: [ qsTr("Colors"), qsTr("Home") ]

    actionBar.leftKeyline: uiState.leftKeyline

    Scrollbar {
        flickableItem: pColors
    }

    Flickable {
        id: pColors
        clip: true

        contentHeight: cColors.implicitHeight

        anchors.fill: parent
        anchors.rightMargin: parent.width < Units.dp(780) ? 0 : (parent.width-Units.dp(780))

        property bool selected: selectedTab == 0
        opacity: selected ? 1 : 0
        anchors.topMargin: selected ? 0 : -Units.dp(8)
        Behavior on opacity { NumberAnimation { duration: 160 } }
        Behavior on anchors.topMargin { NumberAnimation { duration: 180; easing.type: Easing.InOutCubic } }

        Column {
            id: cColors
            anchors {
                fill: parent
                margins: Units.dp(16)
            }

            Label {
                style: "body2"
                text: qsTr("Button and text size")
            }
            Slider {
                property bool canUpdate: false

                anchors.left: parent.left
                anchors.right: parent.right

                minimumValue: 0.75
                maximumValue: 1.6
                stepSize: 0.05
                value: Units.multiplier;

                updateValueWhileDragging: false

                tickmarksEnabled: true

                onValueChanged: {
                    if(canUpdate) {
                        Units.multiplier = value;
                        context.settings.setDpMultiplier(value);
                    }
                }

                Component.onCompleted: canUpdate = true;
            }

            Item { width: 1; height: Units.dp(8) }

            Label {
                style: "body2"
                text: qsTr("Primary color")
            }
            ColorPicker {
                Component.onCompleted: selectedColor = Theme.primaryColor
                property color original: "#00000000"
                onColorHovered: {
                    if(hover) {
                        if(original == "#00000000") original = Theme.primaryColor;
                        Theme.primaryColor = c;
                    } else {
                        if(original != "#00000000") Theme.primaryColor = original;
                    }
                }
                onColorSelected: {
                    Theme.primaryColor = c;
                    original = c;
                    context.settings.primaryColor = c;
                }
            }

            Item { width: 1; height: Units.dp(16) }

            Label {
                style: "body2"
                text: qsTr("Accent color")
            }
            ColorPicker {
                Component.onCompleted: selectedColor = Theme.accentColor
                property color original: "#00000000"
                onColorHovered: {
                    if(hover) {
                        if(original == "#00000000") original = Theme.accentColor;
                        Theme.accentColor = c;
                    } else {
                        if(original != "#00000000") Theme.accentColor = original;
                    }
                }
                onColorSelected: {
                    Theme.accentColor = c;
                    original = c;
                    context.settings.accentColor = c;
                }
            }
//            Item {
//                width: children[0].width - Units.dp(12)
//                height: children[0].height
//                CheckBox {
//                    anchors {
//                        fill: parent
//                        leftMargin: -Units.dp(12)
//                    }
//                    text: qsTr("Dark background")
//                    onCheckedChanged: {
//                        if(checked) {
//                            Theme.backgroundColor = "#424242";
//                        } else {
//                            Theme.backgroundColor = "#f3f3f3";
//                        }
//                    }
//                }
//            }
            Item { width: 1; height: Units.dp(32) }
        }
    }
    Flickable {
        id: pHome
        clip: true

        contentHeight: cHome.implicitHeight

        anchors.fill: parent

        property bool selected: selectedTab == 1
        opacity: selected ? 1 : 0
        anchors.topMargin: selected ? 0 : -Units.dp(8)
        Behavior on opacity { NumberAnimation { duration: 160 } }
        Behavior on anchors.topMargin { NumberAnimation { duration: 180; easing.type: Easing.InOutCubic } }
        visible: opacity > 0

        Column {
            id: cHome
            anchors {
                fill: parent
                topMargin: Units.dp(16)
            }

            Item {
                width: parent.width
                height: children[0].height
                Label {
                    anchors {
                        left: parent.left
                        leftMargin: Units.dp(16)
                        right: parent.right
                        rightMargin: Units.dp(8)
                    }
                    style: "body2"
                    text: qsTr("Home contents") + " (" + qsTr("available in a future version") + ")"
                    wrapMode: Text.WordWrap
                }
            }
            Repeater {
                model: ListModel {
                    ListElement { name: "Welcome message"; description: "Gives a brief introduction to the app." }
                    ListElement { name: "Latest episodes"; description: "Shows the latest episodes in your library." }
                    ListElement { name: "Listen to/watch"; description: "Offers buttons to quickly start listening to or watching something." }
                    ListElement { name: "Known issues"; description: "Lets you know about some possible issues you may encounter using the app." }
                }

                delegate: ListItems.Subtitled {
                    text: name
                    subText: description
                    secondaryItem: CheckBox {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: true
                        enabled: false
                    }
                }
            }
        }
    }
}
