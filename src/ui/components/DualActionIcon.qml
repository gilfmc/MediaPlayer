import QtQuick 2.5
import Material 0.2
import Material.Extras 0.1

/*!
   \qmltype DualActionIcon
   \inqmlmodule Material

   \brief An Icon that holds two actions which can be toggled using showAlternateAction
 */
Item {
    id: dualActionIcon

    property bool showAlternateAction: false
    property bool clockwiseMotion: true

    readonly property Action currentAction: showAlternateAction ? alternateAction : action

    property Action action
    property string iconName
    property string iconSource: action ? action.iconSource : "icon://" + iconName

    property Action alternateAction
    property string alternateIconName
    property string alternateIconSource: alternateAction ? alternateAction.iconSource : "icon://" + iconName

    property color color: '#ffffff'
    property alias size: icon.size

    property int animationDuration: 260

    width: size
    height: size
    enabled: currentAction ? currentAction.enabled : true

    Icon {
        id: icon

        color: parent.color

        anchors.centerIn: parent

        source: parent.iconSource

        rotation: showAlternateAction ? (clockwiseMotion ? +180 : -180) : 0
        Behavior on rotation {
            NumberAnimation { duration: animationDuration }
        }
        opacity: showAlternateAction ? 0 : 1
        Behavior on opacity {
            NumberAnimation { duration: animationDuration }
        }
    }

    Icon {
        id: alternateIcon

        color: parent.color

        anchors.centerIn: parent

        source: parent.alternateIconSource

        rotation: showAlternateAction ? 0 : (clockwiseMotion ? -180 : +180)
        Behavior on rotation {
            NumberAnimation { duration: animationDuration }
        }
        opacity: showAlternateAction ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: animationDuration }
        }
    }
}
