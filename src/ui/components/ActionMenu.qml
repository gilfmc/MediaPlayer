import QtQuick 2.5

import Material 0.2

Action {
    readonly property bool hasMenu: true

    //property list<Action> actions
    default property var actions

    property PopupMenu menu
    onMenuChanged: menu.actions = actions;
}
