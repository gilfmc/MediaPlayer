import QtQuick 2.5
import Material 0.2

Item {
    property alias title: toolbar.title

    property alias backgroundColor: background.backgroundColor
    property alias backgroundOpacity: background.opacity
    property alias elevation: background.elevation

    height: toolbar.height

    property alias backAction: toolbar.backAction
    property alias actions: toolbar.actions

    property alias maxActionCount: toolbar.maxActionCount

    property alias customContent: toolbar.customContent

    anchors {
        left: parent.left
        right: parent.right
    }

    View {
        id: background

        anchors.fill: parent
        elevation: 3
        backgroundColor: Theme.primaryColor

        Behavior on backgroundColor {
            ColorAnimation { duration: 380 }
        }
        Behavior on elevation {
            NumberAnimation { duration: 180 }
        }
        Behavior on opacity {
            NumberAnimation { duration: 320 }
        }
    }

    ActionBar {
        id: toolbar
    }

    Behavior on height {
        NumberAnimation {
            duration: 380
            easing.type: Easing.InOutQuad
        }
    }
}
