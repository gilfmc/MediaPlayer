import QtQuick 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import Material 0.2
import Material.ListItems 0.1

View {
    id: listItem

    width: parent.width
//    anchors {
//        left: parent ? parent.left : undefined
//        right: parent ? parent.right : undefined
//    }

    property int margins: Units.dp(16)

    property bool selected
    property bool interactive: true

    property int dividerInset: actionItem.visible ? listItem.height : 0
    //property bool showDivider: itemType == 0 ? false : contentItem.children.length === 0

    signal clicked()
    signal pressAndHold()
    signal openContextMenu()

    opacity: enabled ? 1 : 0.6

//    ThinDivider {
//        anchors.bottom: parent.bottom
//        anchors.leftMargin: dividerInset

//        visible: showDivider
//    }

    Ink {
        id: ink

        onClicked: { if(mouse.button === Qt.LeftButton) listItem.clicked(); else if(mouse.button === Qt.RightButton) listItem.openContextMenu(); }
        onPressAndHold: listItem.pressAndHold()

        anchors.fill: parent

        enabled: listItem.interactive && listItem.enabled
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        z: -1
    }

    tintColor: selected
               ? Qt.rgba(0,0,0,0.05)
               : ink.containsMouse ? Qt.rgba(0,0,0,0.03) : Qt.rgba(0,0,0,0)

    property int itemType: 0

    implicitHeight: itemType == 0 ? Units.dp(48) : Units.dp(40)
    height: implicitHeight

    property alias text: label.text
    property alias valueText: valueLabel.text

    property alias action: actionItem.children
    property alias iconName: icon.name
    property alias iconSource: icon.source
    property alias secondaryItem: secondaryItem.children
    property alias content: contentItem.children

    property alias itemLabel: label
    property alias itemValueLabel: valueLabel

    property alias textColor: label.color
    property alias iconColor: icon.color

    //dividerInset: actionItem.visible ? listItem.height : 0

    //interactive: itemType == 0 ? false : contentItem.children.length === 0

    implicitWidth: {
        var width = listItem.margins * 2

        if (actionItem.visible)
            width += actionItem.width + row.spacing

        if (contentItem.visible)
            width += contentItem.implicitWidth + row.spacing
        else
            width += label.implicitWidth + row.spacing

        if (valueLabel.visible)
            width += valueLabel.width + row.spacing

        if (secondaryItem.visible)
            width += secondaryItem.width + row.spacing

        return width
    }

    RowLayout {
        id: row
        anchors.fill: parent

        anchors.leftMargin: listItem.margins
        anchors.rightMargin: listItem.margins

        spacing: Units.dp(16)

        Item {
            id: actionItem

            Layout.preferredWidth: Units.dp(32)
            Layout.preferredHeight: width
            Layout.alignment: Qt.AlignCenter

            //visible: children.length > 1 || icon.valid

            Icon {
                id: icon

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                }

                visible: valid
                color: listItem.selected ? Theme.primaryColor : Theme.light.iconColor
                size: Units.dp(24)
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: parent.height

            Item {
                id: contentItem

                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                visible: children.length > 0
            }

            Label {
                id: label

                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                elide: Text.ElideRight
                style: itemType == 0 ? "subheading" : "body1"

                color: listItem.selected ? Theme.primaryColor : Theme.light.textColor

                visible: !contentItem.visible
            }
        }

        Label {
            id: valueLabel

            Layout.alignment: Qt.AlignVCenter

            color: Theme.light.subTextColor
            elide: Text.ElideRight
            style: "caption"

            visible: text != ""
        }

        Item {
            id: secondaryItem

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: childrenRect.width
            Layout.preferredHeight: parent.height

            visible: children.length > 0
        }
    }

    IconButton {
        id: contextMenu
        opacity: ink.containsMouse || children[0].containsMouse ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        width: Units.dp(32)
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }

        onClicked: listItem.openContextMenu();
        //Layout.topMargin: Units.dp(1)
        //Layout.preferredHeight: parent.height

        iconName: "navigation/more_vert"
        size: Units.dp(20)
        color: "#40000000"
    }
}
