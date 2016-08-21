import QtQuick 2.5
import QtQuick.Controls 1.4

import Material 0.2

Grid {
    id: colorPicker
    property var colorNames: [
        "red", "pink", "purple", "deepPurple", "indigo",
        "blue", "lightBlue", "cyan", "teal", "green",
        "lightGreen", "lime", "yellow", "amber", "orange",
        "deepOrange", "grey", "blueGrey", "brown"
    ]
    property var colorVariants: [ "300", "500", "800", "A400" ]
    property string falbackVariant: "200"

    property color selectedColor

    signal colorSelected(color c)
    signal colorHovered(color c, bool hover)

    anchors {
        left: parent.left
        right: parent.right
    }

    columns: width < Units.dp(360) ? 2 : (width < Units.dp(720) ? 3 : 6)
    spacing: width < Units.dp(600) ? Units.dp(6) : Units.dp(8)

    property int realColumnCount: colorPicker.colorVariants.length * colorPicker.columns
    property int baseWidth: width - columns * 4 * spacing - (colorVariants.length+2) * Units.dp(8)

    Repeater {
        model: colorPicker.colorNames

        Rectangle {
           width: children[0].width + Units.dp(8)
           height: children[0].height + Units.dp(8)
           radius: Units.dp(2)
           color: Palette.colors[modelData]['50']

           Row {
               anchors.centerIn: parent
               spacing: Units.dp(8)
               Repeater {
                   id: rColorGroup
                   property string currentColorName: modelData
                   model: colorPicker.colorVariants
                   Rectangle {
                       width: colorPicker.baseWidth / colorPicker.realColumnCount
                       height: width
                       radius: Units.dp(2)
                       color: Palette.colors[rColorGroup.currentColorName][modelData] ? Palette.colors[rColorGroup.currentColorName][modelData] : Palette.colors[rColorGroup.currentColorName][colorPicker.falbackVariant]
                       Ink {
                           anchors.fill: parent
                           hoverEnabled: true
                           onHoveredChanged: {
                               colorHovered(parent.color, containsMouse)
                           }
                           onClicked: {
                               selectedColor = parent.color;
                               colorSelected(parent.color);
                           }
                       }
                   }
               }
           }
        }
    }

    Object {
        Icon {
            id: cursor
            anchors.fill: parent
            visible: parent != null
            name: "navigation/check"
            color: parent && Theme.isDarkColor(parent.color) ? Theme.dark.iconColor : Theme.light.iconColor
        }
    }

    onSelectedColorChanged: {
        if(selectedColor) {
            var len = children.length;
            for(var i = 0; i < len; i++) {
                var squares = children[i].children[0].children;
                var len2 = squares.length;
                for(var j = 0; j < len2; j++) {
                    if(squares[j].color === selectedColor) {
                        cursor.parent = squares[j];
                        return;
                    }
                }
            }
        }
        cursor.parent = null;
    }
}
