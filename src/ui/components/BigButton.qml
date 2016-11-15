import QtQuick 2.5

import Material 0.2

Item {
    id: root

    width: parent.width/columnCount
    height: Units.dp(48)

    Card {
        id: card
        backgroundColor: Theme.accentColor
        property bool isDarkBackground: Theme.isDarkColor(backgroundColor)
        elevation: ink.containsMouse ? 1 : 0

        anchors {
            fill: parent
            margins: Units.dp(4)
        }

        Icon {
            id: icon
            anchors {
                left: parent.left
                leftMargin: Units.dp(8)
                verticalCenter: parent.verticalCenter
            }
            name: _icon;
            color: card.isDarkBackground ? Theme.dark.iconColor : Theme.light.iconColor
        }

        Label {
            anchors {
                left: icon.right
                leftMargin: Units.dp(8)
                top: parent.top
                bottom: parent.bottom
            }
            text: _text
            style: "subheading"
            color: card.isDarkBackground ? Theme.dark.textColor : Theme.light.textColor
            verticalAlignment: Text.AlignVCenter
        }

        Ink {
            id: ink
            anchors.fill: parent
            onClicked: _listener.onItemClick(index);
        }
    }
}
