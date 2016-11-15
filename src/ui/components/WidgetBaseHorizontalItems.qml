import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import Material 0.2

import org.papyros.mediaplayer 1.0

import "../components"

Card {
    id: root

    property var homeUi

    property var title
    property int type
    property var model
    onModelChanged: {
        model.itemLimit = Qt.binding(function() { return repeater.itemCount });
        repeater.model = model;
    }

    anchors{
        left: parent.left
        right: parent.right
    }

    height: actionBar.height + rowBar.height

    Item {
        id: actionBar

        height: Units.dp(56)

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Label {
            text: root.title
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

    Row {
        id: rowBar
        anchors {
            top: actionBar.bottom
            left: parent.left
            right: parent.right
        }

        Repeater {
            id: repeater

            property int baseSize: Units.dp(root.width > Units.dp(620) ? (root.width > Units.dp(800) ? 272 : 240) : 192)
            property int itemCount: Math.max(1, Math.round(root.width / baseSize))
            property int itemWidth: (root.width-Units.dp(8)/(itemCount-1))/itemCount  // (width - 5*Units.dp(8))/5
            property int itemHeight: 0.58 * itemWidth

            delegate: type == 1 ? thumbnail : undefined

            Component {
                id: thumbnail

                Item {
                    visible: index < repeater.itemCount
                    width: repeater.itemWidth
                    height: repeater.itemHeight

                    MediaThumbnail {
                        onClick: root.model.onMediaContentClick(index, false);
                        onPlayButtonClick: root.model.onMediaContentClick(index, true);

                        visible: parent.visible

                        anchors {
                            fill: parent
                            leftMargin: Units.dp(8)
                            bottomMargin: Units.dp(8)
                        }

                        elevation: 0

                        mediaId: id
                        mediaTitle: title
                        mediaDescription: description
                        mediaLength: length
                        mediaDate: date
                    }
                }
            }
        }
    }
}
