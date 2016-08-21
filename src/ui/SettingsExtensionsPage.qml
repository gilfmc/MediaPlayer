import QtQuick 2.5

import Material 0.2
import Material.ListItems 0.1 as ListItems

Page {
    title: qsTr("Extensions")

//    actions: [
//        Action {
//            text: "Help"
//            iconName: "action/help"
//        }
//    ]

    Scrollbar {
        flickableItem: lvExtensions
    }

    ListView {
        id: lvExtensions
        model: context.pluginManager
        delegate: ListItems.Subtitled {
            interactive: type != 0
            height: Units.dp(window.height > 12*Units.gridUnit ? (type == 0 ? 64 : 72) : (type == 0 ? 56 : 64))
            text: name
            subText: description
            iconName: interactive ? "action/extension" : ""
        }
        anchors.fill: parent
    }
}
