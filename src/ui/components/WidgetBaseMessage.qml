import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import Material 0.2

Card {
    id: root

    property var homeUi
    property var model

    property alias title: lTitle.text
    property alias message: lMessage.text

    property color textColor: Theme.light.textColor

//    property bool lightText
//    onLightTextChanged: {
//        if(lightText) {
//            lTitle.color = Theme.dark.textColor;
//            lMessage.color = Theme.dark.textColor;
//        } else {
//            lTitle.color = Theme.light.textColor;
//            lMessage.color = Theme.light.textColor;
//        }
//    }

    anchors{
        left: parent.left
        right: parent.right
    }

    height: actionBar.height + lMessage.height /* column.height*/ + Units.dp(8)

//    Column {
//        id: column

//        anchors {
//            top: parent.top
//            left: parent.left
//            right: parent.right
//        }

        Item {
            id: actionBar

            height: Units.dp(56)

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }

            Label {
                id: lTitle
                style: "title"
                anchors {
                    left: parent.left
                    leftMargin: Units.dp(16)
                    verticalCenter: parent.verticalCenter
                }
            }

            IconButton {
                id: bMenu
                anchors {
                    right: parent.right
                    rightMargin: Units.dp(16)
                    verticalCenter: parent.verticalCenter
                }

                action: Action {
                    //text: "Card options"
                    iconName: "navigation/more_vert"
                    onTriggered: homeUi.openCardMenu(root, bMenu, Units.dp(8), -Units.dp(8))
                }
            }
        }

        Label {
            id: lMessage

            anchors {
                top: actionBar.bottom
                topMargin: -Units.dp(8)
                left: parent.left
                right: parent.right
                leftMargin: Units.dp(16)
                rightMargin: Units.dp(16)
            }

            style: "subheading"
            color: textColor

            wrapMode: Text.Wrap
        }
//    }
}
