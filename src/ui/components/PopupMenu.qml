import QtQuick 2.5
import QtQuick.Layouts 1.1
import Material 0.2
import Material.ListItems 0.1 as ListItem

Dropdown {
    id: popupMenu
    objectName: "popupMenu"

    width: Units.dp(240)
    height: columnView.height + Units.dp(16)

    property list<Action> actions

    ColumnLayout {
        id: columnView
        width: parent.width
        anchors.centerIn: parent

        Repeater {
            id: repeater
            model: actions.length

            ListItem.Standard {
                id: listItem

                objectName: "action/" + action.objectName

                property Action menuAction: actions[index]
//                onMenuActionChanged: {
//                    if(menuAction.checkable) {
//                        var checkbox = componentCheckbox.createObject(listItem, {"listItem": listItem, "checked": menuAction.checked});
//                        action[1] = checkbox;
//                    }
//                }

                visible: menuAction.visible

                text: menuAction.name
                iconSource: menuAction.checkable ? "icon://navigation/check" : menuAction.iconSource
                iconColor: menuAction.checkable && !menuAction.checked ? "transparent" : Theme.light.iconColor
                enabled: menuAction.enabled

                onClicked: {
                    if(menuAction.checkable) {
                        menuAction.checked = !menuAction.checked;
                    }
                    menuAction.triggered(listItem);
                    popupMenu.close();
                }

//                Component.onCompleted: {
//                    if(menuAction.hasMenu) {
//                        Qt.createQmlObject("import QtQuick 2.5; MouseArea { anchors.fill:parent; hoverEnabled:true; onContainsMouseChanged:{ if(containsMouse) openSubmenu(listItem); } }", listItem);
//                    }
//                }
            }
        }
    }

//    function openSubmenu(item) {
//        if(item.menuAction.menu) item.menuAction.menu.open(item, -item.width, 0);
//        else {
//            var menu = Qt.createQmlObject("import Material 0.2; PopupMenu {}", item, "blahhh");
//            item.menuAction.menu = menu;
//            menu.open(item, -item.width, 0);
//        }
//    }

//    Component {
//        id: componentCheckbox
//        Icon {
//            property var listItem
//            property bool checked
//            name: checked ? "navigation/check" : ""

//            anchors {
//                verticalCenter: parent.verticalCenter
//                left: parent.left
//            }

//            color: listItem.selected ? Theme.primaryColor : Theme.light.iconColor
//            size: Units.dp(24)
//        }
//    }
}
