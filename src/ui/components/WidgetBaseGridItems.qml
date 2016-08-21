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

    property var model
    onModelChanged: {
        //model.itemLimit = Qt.binding(function() { return repeater.itemCount });
        gridView.delegate = model.component;
        gridView.model = model;
    }

    anchors{
        left: parent.left
        right: parent.right
    }

    height: actionBar.height + gridView.height

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

    GridView {
        id: gridView

        property int margin: Units.dp(4)

        height: {
            if(model) {
                var result = 0;
                var len = children.length;
                for(var i = 0; i < len; i+=model.columnCount()) {
                    result += children[i].height;
                }
                return result;
            } else {
                return 0;
            }
        }

        anchors {
            top: actionBar.bottom
            topMargin: -margin
            left: parent.left
            leftMargin: margin
            right: parent.right
            rightMargin: margin
        }

        cellWidth: width/model.columnCount()
        cellHeight: Units.dp(48)
        //rows: repeater.model ? repeater.model.rowCount() : 0
        //columns: repeater.model ? repeater.model.columnCount() : 0

//        Repeater {
//            id: repeater
//        }
    }
}
