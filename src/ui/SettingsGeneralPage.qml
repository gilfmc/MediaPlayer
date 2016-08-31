import QtQuick 2.5
import QtQuick.Controls 1.4

import Material 0.2

Page {
    title: qsTr("General")

    Column {
        anchors.fill: parent

        Item {
            width: parent.width
            implicitHeight: lRestoreState.implicitHeight + Units.dp(24)
            height: lRestoreState.implicitHeight + Units.dp(24)
            Label {
                id: lRestoreState
                style: "body2"
                text: qsTr("When the app opens")
                anchors {
                    fill: parent
                    margins: Units.dp(16)
                    bottomMargin: Units.dp(8)
                }
            }
        }
        ExclusiveGroup { id: bRestoreState }
        RadioButton {
            exclusiveGroup: bRestoreState
            text: qsTr("Keep everything as I left")
            checked: context.settings.restoreState
            onCheckedChanged: context.settings.restoreState = checked
        }
        RadioButton {
            exclusiveGroup: bRestoreState
            text: qsTr("Start with an empty playlist")
            checked: !context.settings.restoreState
        }
    }
}
