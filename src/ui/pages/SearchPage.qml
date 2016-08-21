import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Material 0.2
import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

NavigatorPage {
    id: root

    readonly property bool isSearch: true
    property bool largeItems: root.width > Units.dp(800)

    title: qsTr("Search", "Title of the page, not the action itself.")

    function search(text) {
        cpp.search(text);
    }

    Rectangle {
        anchors.fill: parent
        visible: uiState && uiState.videoBehind && uiState.osdOpen
        color: "#4affffff"
    }

    ListView {
        anchors.fill: parent
        topMargin: toolbar.height
        bottomMargin: uiState && uiState.videoBehind && uiState.osdOpen ? uiState.bottomBarHeight + Units.dp(8) : Units.dp(8)
        model: SearchUi {
            id: cpp
            context: _context
        }
        delegate: Item {
            id: contents

            visible: opacity != 0
            opacity: count > 0 ? 1 : 0
            width: root.width
            height: visible ? group.height + items.height : 0

            Behavior on height { NumberAnimation { duration: 260; easing.type: Easing.InOutCubic } }
            Behavior on opacity { NumberAnimation { duration: 240; } }

            Item {
                id: group
                width: parent.width - root.rightMargin
                height: Math.max(groupTitle.implicitHeight, bMore.implicitHeight) + Units.dp(20)

                Label {
                    id: groupTitle
                    text: title
                    style: "headline"
                    anchors {
                        leftMargin: Units.dp(16)
                        left: parent.left
                        bottom: parent.bottom
                        bottomMargin: Units.dp(8)
                    }
                }

                Button {
                    id: bMore
                    visible: false
                    text: "More"
                    //backgroundColor: Theme.accentColor
                    anchors {
                        rightMargin: Units.dp(16)
                        right: parent.right
                        bottom: parent.bottom
                        bottomMargin: Units.dp(8)
                    }
                    onClicked: {
                        var page = openPage("MediaListPage");
                        page.title = groupTitle.text;
                        page.ui = ui.model.getListUiProvider(i);
                    }
                }
            }

            Loader {
                id: items
                anchors.top: group.bottom
                //asynchronous: true
                sourceComponent: (type == 1 ? viewType1 : (type == 2 ? viewType2 : (type == 3 ? viewType3 : (type == 4 ? viewType4 : undefined))))
            }

            Component {
                id: viewType1
            Column {
                id: simpleListItem
                property bool showMediaNumber: false // showNumber ? true : false

                width: root.width
                height: count > 0 ? implicitHeight : 0

                Repeater {
                    //model: visible ? ui.model.getGroupItem(i) : 0
                    Component.onCompleted: model = cpp.getGroup(index);

                    delegate: ListItems.BaseListItem {
                        id: listItem
                        height: Units.dp(52)
                        onClicked: context.playlist.addMedia(id);

                        RowLayout {
                            id: row
                            anchors.fill: parent

                            anchors.leftMargin: simpleListItem.showMediaNumber ? listItem.margins : 0
                            anchors.rightMargin: listItem.margins + root.rightMargin

                            //spacing: Units.dp(16)

                            Label {
                                visible: simpleListItem.showMediaNumber
                                style: "subheading"
                                text: simpleListItem.showMediaNumber ? number : ""

//                                        width: width/2
//                                        height: listItem.height

//                                        horizontalAlignment: Text.Center
//                                        verticalAlignment: Text.Center

                                Layout.preferredWidth: width
                                Layout.preferredHeight: height
                                Layout.maximumWidth: width
                                Layout.minimumWidth: width

                                //Rectangle { anchors.fill: parent; color: "#88123456"; }
                            }
                            IconButton {
                                id: actionItem

                                width: listItem.height
                                height: width

                                Layout.preferredWidth: listItem.height
                                Layout.preferredHeight: listItem.height
                                Layout.maximumWidth: Layout.preferredWidth
                                Layout.minimumWidth: Layout.preferredWidth

                                //Rectangle { anchors.fill: parent; color: "#88123456"; }

                                //Icon {
                                    //id: icon

                                    iconName: "av/play_arrow"

                                    //anchors.centerIn: parent

                                    //visible: valid
                                    color: listItem.selected ? Theme.primaryColor : Theme.light.iconColor
                                    size: Units.dp(24)

                                    onClicked: {
                                        context.playlist.addMedia(id);
                                        context.playlist.play(context.playlist.rowCount()-1);
                                    }
                                //}
                            }
                            Label {
                                style: "subheading"
                                text: title

                                //horizontalAlignment: Text.Left
                                //Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                            Label {
                                style: "subheading"
                                text: description
                                elide: Text.ElideRight
                            }
                            Label {
                                style: "subheading"
                                text: length
                                horizontalAlignment: Text.AlignRight
                                Layout.preferredWidth: Units.dp(48)
                                Layout.minimumWidth: Layout.preferredWidth
                            }
                        }
                    }
                }
            }
            }

            Component {
                id: viewType2
                Row {
                    id: coversRow
                    width: root.width - Units.dp(8)
                    height: count > 0 ? itemHeight : 0

                    property int groupIndex: index
                    property int maxItems: width/Units.gu(2.5)
                    property int itemWidth: width/maxItems
                    property int itemHeight: itemWidth*1.48

                    Repeater {
                        Component.onCompleted: {
                            var m = cpp.getGroup(index);
                            m.itemLimit = Qt.binding(function(){ return coversRow.maxItems; });
                            model = m;
                        }
                        delegate: Item {
                            visible: index < maxItems
                            width: coversRow.itemWidth
                            height: coversRow.itemHeight

                            Card {
                                anchors {
                                    fill: parent
                                    margins: Units.dp(8)
                                }

                                Image {
                                    id: image

                                    anchors {
                                        left: parent.left
                                        top: parent.top
                                        right: parent.right
                                        rightMargin: -1
                                        bottom: parent.bottom
                                        bottomMargin: -1
                                    }

                                    source: picture
                                    //asynchronous: true
                                    fillMode: Image.PreserveAspectCrop

                                    sourceSize.width: Units.dp(192)
                                }
                                LinearGradient {
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    height: label.height * 3

                                    cached: true

                                    start: Qt.point(0, height)
                                    end: Qt.point(0, 0)

                                    gradient: Gradient {
                                        GradientStop { position: 0.00; color: "#66000000" } // #33
                                        GradientStop { position: 0.33; color: "#32000000" } // #19
                                        GradientStop { position: 1.00; color: "#00000000" }
                                    }
                                }
                                LinearGradient {
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    height: menuButton.height * 3

                                    cached: true

                                    start: Qt.point(0, 0)
                                    end: Qt.point(0, height)

                                    gradient: Gradient {
                                        GradientStop { position: 0.00; color: "#66000000" } // #33
                                        GradientStop { position: 0.33; color: "#32000000" } // #19
                                        GradientStop { position: 1.00; color: "#00000000" }
                                    }
                                }
                                Label {
                                    id: label

                                    anchors.left: parent.left
                                    anchors.leftMargin: largeItems ? Units.dp(16) : Units.dp(8)
                                    anchors.right: parent.right
                                    anchors.rightMargin: largeItems ? Units.dp(16) : Units.dp(8)
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: largeItems ? Units.dp(18) : Units.dp(9)

                                    elide: Text.ElideRight
                                    style: largeItems ? "title" : "subheading"
                                    font.weight: Font.DemiBold
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                    lineHeight: 0.8
                                    color: Theme.dark.textColor

                                    text: title
                                }
                                Ink {
                                    id: ink
                                    onClicked: openPage("MediaContentPage").ui = cpp.getItemUi(groupIndex, index);
                                    anchors.fill: parent
                                    color: Qt.rgba(1,1,1,0.1)

                                    z: 1
                                }
                                IconButton {
                                    id: menuButton

                                    anchors.right: parent.right
                                    anchors.rightMargin: largeItems ? Units.dp(8) : Units.dp(4)
                                    anchors.top: parent.top
                                    anchors.topMargin: largeItems ? Units.dp(8) : Units.dp(4)

                                    size: largeItems ? Units.dp(18) : Units.dp(16)

                                    iconName: "navigation/more_vert"
                                    color: Theme.dark.iconColor
                                    opacity: 0.7

                                    z: 2
                                }
                            }
                        }
                    }
                }
            }

            Component {
                id: viewType4
            Row {
                id: thumbnailItems
                width: root.width - Units.dp(8)
                height: count > 0 ? itemHeight : 0

                spacing: Units.dp(8)
                anchors.left: parent.left
                anchors.leftMargin: Units.dp(8)

                property int baseSize: Units.dp(root.width > Units.dp(620) ? (root.width > Units.dp(800) ? 272 : 240) : 192)
                property int itemCount: Math.max(1, Math.round(width / baseSize))
                property int itemWidth: (width-itemCount*Units.dp(8))/itemCount  // (width - 5*Units.dp(8))/5
                property int itemHeight: 0.58 * itemWidth

//                move: Transition {
//                    NumberAnimation { properties: "x"; from: items.width; duration: 600; easing.type: Easing.OutCubic }
//                }

                Repeater {
                    //model: parent.itemCount
                    Component.onCompleted: {
                        var m = cpp.getGroup(index);
                        m.itemLimit = Qt.binding(function(){ return thumbnailItems.itemCount; });
                        model = m;
                    }

                    delegate: MediaThumbnail {
                        visible: index < thumbnailItems.itemCount
                        width: parent.itemWidth
                        height: parent.itemHeight

                        mediaId: id
                        mediaTitle: title
                        mediaDescription: description
                        mediaLength: length
                        mediaDate: date

                        onClick: context.playlist.addMedia(id);
                        onPlayButtonClick: {
                            context.playlist.addMedia(id);
                            context.playlist.play(context.playlist.rowCount()-1);
                        }
                    }
                }
            }
            }

            Component {
                id: viewType3
                Row {
                    id: peopleRow
                    width: root.width - Units.dp(8)
                    height: count > 0 ? itemHeight : 0

                    property int groupIndex: index
                    property int maxItems: width/Units.gu(2.5)
                    property int itemWidth: width/maxItems
                    property int itemHeight: itemWidth + Units.dp(24)

                    Repeater {
                        Component.onCompleted: {
                            var m = cpp.getGroup(index);
                            m.itemLimit = Qt.binding(function(){ return peopleRow.maxItems; });
                            model = m;
                        }
                        delegate: Item {
                            visible: index < maxItems
                            width: peopleRow.itemWidth
                            height: peopleRow.itemHeight

                            Ink {
                                id: ink
                                onClicked: openPage("MediaContentPage").ui = cpp.getItemUi(groupIndex, index);
                                anchors.fill: parent
                            }

                            CircleImage {
                                id: image

                                property int size: peopleRow.itemWidth - Units.dp(32)

                                width: size
                                height: size

                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                    top: parent.top
                                    topMargin: parent.height/2 - height/2 - label.height/label.lineCount/2
                                }

                                elevation: 1

                                source: picture
                                //asynchronous: true
                                fillMode: Image.PreserveAspectCrop

                                sourceSize.width: Units.gu(2.5)
                                sourceSize.height: Units.gu(2.5)
                            }
                            Label {
                                id: label

                                anchors.top: image.bottom
                                anchors.topMargin: Units.dp(4)
                                anchors.left: parent.left
                                anchors.right: parent.right

                                elide: Text.ElideRight
                                style: "subheading"
                                horizontalAlignment: Text.Center
                                wrapMode: Text.Wrap
                                maximumLineCount: 2
                                color: Theme.light.textColor

                                text: title
                            }
                        }
                    }
                }
            }
        }
//        displaced: Transition {
//            NumberAnimation { properties: "y"; duration: 200; easing.type: Easing.InOutCubic }
//        }
//        move: Transition {
//            NumberAnimation { properties: "y"; duration: 200; easing.type: Easing.InOutCubic }
//        }
    }

//    Label {
//        style: "caption"
//        anchors.centerIn: parent
//        text: qsTr("Looking for %1...").arg(query)
//    }
}
