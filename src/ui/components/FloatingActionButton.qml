import QtQuick 2.5
import Material 0.2
import Material.Extras 0.1

/*!
   \qmltype FloatingActionButton
   \inqmlmodule Material

   \brief A big circular button, usually represents the most important action on the interface.
 */
Card {
    id: iconButton

    backgroundColor: Theme.accentColor

    property alias showAlternateAction: icon.showAlternateAction
    property alias clockwiseMotion: icon.clockwiseMotion

    property alias currentAction: icon.currentAction

    property alias action: icon.action
    property alias iconName: icon.iconName
    property alias iconSource: icon.iconSource

    property alias alternateAction: icon.alternateAction
    property alias alternateIconName: icon.alternateIconName
    property alias alternateIconSource: icon.alternateIconSource

    property alias color: icon.color
    property alias size: icon.size

    property bool mini: false
    property bool showTooltip: true

    property alias drag: ink.drag

    property alias animationDuration: icon.animationDuration

    property var snackbar

    signal clicked
    signal pressAndHold
    signal released

    width: mini ? Units.dp(40) : Units.dp(56)
    height: width
    radius: width/2
    enabled: action ? action.enabled : true
    opacity: enabled ? 1 : 0.6

    elevation: ink.containsMouse ? 3 : 1

    onClicked: {
        if (action) action.triggered(icon)
    }

    anchors {
        bottom: parent.bottom
        bottomMargin: snackbar && snackbar.opened ? Units.dp(16) + snackbar.height : Units.dp(16)
        right: parent.right
        rightMargin: Units.dp(16)

        Behavior on bottomMargin {
            // TODO: should follow "better" the Material Design guidelines
            SpringAnimation { spring: 2; damping: 0.2 }
        }
    }

    Ink {
        id: ink

        anchors.fill: parent
        enabled: iconButton.enabled
        circular: true

//        width: parent.width
//        height: parent.height

//        z: 0

        onClicked: {
            iconButton.clicked()
        }

//        onPressAndHold: iconButton.pressAndHold(mouse);

//        onReleased: iconButton.released(mouse);
    }

    Connections {
        target: ink

        onReleased: iconButton.released(mouse);

        onPressAndHold: iconButton.pressAndHold(mouse);
    }

    DualActionIcon {
        id: icon
        color: Theme.isDarkColor(backgroundColor) ? Theme.dark.iconColor : Theme.light.iconColor
        anchors.centerIn: parent
    }

    Tooltip {
        enabled: true
        id: tooltip
        text: currentAction ? currentAction.name : ""
        mouseArea: showTooltip ? ink : undefined
    }
}
