import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import Material 0.2
import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

NavigatorPage {
    id: root

    title: ui ? ui.name : ""
    //bigPictureMode: true
    bigPictureMode: hasPicture ? listView.contentY /*+ listView.headerItem.height*/ - Units.dp(24) - listView.height < -toolbar.height : false
    //toolbarHeight: header.height + header.y

    property MediaContentUiProvider ui;
    property bool hasPicture: ui ? (ui.posterUrl.length > 0 ? true : false) : false

    onUiChanged: {
        if(ui) {
            repeater.model = ui.model;
        }
    }

    Image {
        id: picture

        y: {
            //console.log(Math.max(0, Math.min(1, (listView.contentY+listView.headerItem.height)/(listView.contentHeight-listView.height))));
            return Math.max(0, Math.min(1, (listView.contentY/*+listView.headerItem.height*/)/(listView.contentHeight-listView.height))) * (parent.height - height)
        }

        property int realWidth;
        property int realHeight;

        onStatusChanged: {
            if(status == Image.Ready) {
                realWidth = width;
                realHeight = height;

                width = Qt.binding(function() { return parent.width; });
                height = Qt.binding(function() { return parent.width * realHeight / realWidth; });
                //console.log("(" + realWidth + "x" + realHeight + ") => (" + width + "x" + height + ")");
            }
        }

//        anchors {
//            left: parent.left
//            right: parent.right
//            top: parent.top
//        }

        asynchronous: true
        source: hasPicture ? ui.posterUrl : ""
        //fillMode: Image.PreserveAspectCrop
        //horizontalAlignment: Image.AlignTop

    }

    LinearGradient {
        opacity: hasPicture ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: Units.gridUnit * 3

        //cached: true

        start: Qt.point(0, 0)
        end: Qt.point(0, height)

        gradient: Gradient {
            GradientStop { position: 0.00; color: "#66000000" } // #33
            GradientStop { position: 0.33; color: "#32000000" } // #19
            GradientStop { position: 1.00; color: "#00000000" }
        }
    }

    Card {
        y: header.height - listView.contentY
        width: root.width
        height: root.height - y
        backgroundColor: hasPicture ? "#aaffffff" : "#ffffffff"
        fullWidth: true
        elevation: 2
    }

    Flickable {
        id: listView
        anchors.fill: parent
        contentHeight: listContent.height
        bottomMargin: uiState && uiState.videoBehind && uiState.osdOpen ? uiState.bottomBarHeight : Units.dp(8)
        contentY: root.height - Units.gridUnit // header.height - root.height + Units.dp(80)

        Column {
            id: listContent

            Item {
                id: header
                width: root.width - root.rightMargin
                height: Units.dp(80) + topPadding + cardExtraInfo.y + cardExtraInfo.height
                Behavior on height {
                    NumberAnimation { id: cardHeightAnimation; property bool enabled: false; duration: enabled ? 360 : 0; easing.type: Easing.OutQuad; onStopped: enabled = false; }
                }

                property int topPadding: root.height //Math.max(Units.dp(80), root.height - (cardDescription.height + cardTitle.height + Units.dp(216))) // root.height - (parent.parent.height + contents.height);

                Card {
                    id: card
                    clipContent: false

                    anchors {
                        fill: parent
                        topMargin: card.parent.topPadding
                        bottomMargin: Units.dp(64)
                        leftMargin: Units.dp(64)
                        rightMargin: Units.dp(64)
                    }

                    Label {
                        id: cardTitle
                        style: "title"
                        text: title
                        anchors {
                            top: parent.top
                            topMargin: Units.dp(16)
                            horizontalCenter: parent.horizontalCenter
                        }
                    }
                    Label {
                        id: cardDescription
                        style: "subheading"
                        text: ui ? ui.description : ""
                        horizontalAlignment: Text.AlignJustify
                        wrapMode: Text.WordWrap
                        maximumLineCount: 5
                        anchors {
                            top: cardTitle.bottom
                            topMargin: Units.dp(8)
                            left: parent.left
                            leftMargin: Units.dp(32)
                            right: parent.right
                            rightMargin: Units.dp(32)
                        }
                    }
                    Item {
                        visible: cardDescription.lineCount > 3
                        height: cardShowMoreDescription.height * 1.5
                        anchors {
                            bottom: cardDescription.bottom
                            bottomMargin: -Units.dp(7)
                            left: parent.left
                            right: parent.right
                        }
                        LinearGradient {
                            anchors.fill: parent

                            cached: true

                            start: Qt.point(0, 0)
                            end: Qt.point(0, height)

                            gradient: Gradient {
                                GradientStop { position: 0.00; color: "#00ffffff" }
                                GradientStop { position: 0.33; color: "#88ffffff" }
                                GradientStop { position: 1.00; color: "#ffffffff" }
                            }
                        }
                        Button {
                            id: cardShowMoreDescription
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.leftMargin: Units.dp(32)
                            anchors.right: parent.right
                            anchors.rightMargin: Units.dp(32)
                            text: qsTr("Show more")
                            onClicked: {
                                cardHeightAnimation.enabled = true;
                                cardShowMoreDescription.parent.visible = false;
                                cardDescription.maximumLineCount = undefined;
                            }
                        }
                    }
                    Rectangle {
                        height: 1
                        color: "#33000000"
                        anchors {
                            top: cardDescription.bottom
                            topMargin: Units.dp(16)
                            left: parent.left
                            leftMargin: Units.dp(32)
                            right: parent.right
                            rightMargin: Units.dp(32)
                        }
                    }
                    Item {
                        id: cardExtraInfo
                        height: cardExtra1.height
                        anchors {
                            top: cardDescription.bottom
                            topMargin: Units.dp(28)
                            left: parent.left
                            leftMargin: Units.dp(32)
                            right: parent.right
                            rightMargin: Units.dp(32)
                        }

                        Label {
                            id: cardExtra1
                            width: parent.width/3
                            style: "caption"
                            font.capitalization: Font.AllUppercase
                            text: "other info"
                            anchors.left: parent.left
                        }
                        Label {
                            id: cardExtra2
                            width: cardExtra1.width
                            style: "caption"
                            font.capitalization: Font.AllUppercase
                            text: "date"
                            horizontalAlignment: Text.AlignHCenter
                            anchors.left: cardExtra1.right
                        }
                        Label {
                            id: cardExtra3
                            width: cardExtra1.width
                            style: "caption"
                            font.capitalization: Font.AllUppercase
                            text: "genre"
                            horizontalAlignment: Text.AlignRight
                            anchors.left: cardExtra2.right
                        }
                    }

                    FloatingActionButton {
                        anchors.right: parent.right
                        anchors.rightMargin: -width/2
                        anchors.top: parent.top
                        anchors.topMargin: Units.dp(16)
                        anchors.bottom: undefined
                        //anchors.bottom: parent.bottom
                        //anchors.bottomMargin: -height/2
                        mini: true

                        action: Action {
                            text: qsTr("Play all")
                            iconName: "av/play_arrow"
                            onTriggered: uiState.playAll(ui.mediaType, ui.mediaId);
                        }
                    }
                }
            }

            Repeater {
                id: repeater

                //model: ui ? ui.model : null

                delegate: Column {
                    id: contents

                    width: root.width

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
                        //asynchronous: true
                        sourceComponent: ui.model.getGroupItem(i).rowCount() === 0 ? viewType0 : (type == 1 ? viewType1 : (type == 2 ? viewType2 : undefined))
                    }

                    Component {
                        id: viewType1
                    Column {
                        id: simpleListItem
                        property int groupIndex: i
                        property bool showMediaNumber: showNumber ? true : false

                        width: root.width

                        Repeater {
                            //model: visible ? ui.model.getGroupItem(i) : 0
                            Component.onCompleted: model = ui.model.getGroupItem(i);

                            delegate: ListItems.BaseListItem {
                                id: listItem
                                height: Units.dp(52)
                                onClicked: ui.model.addToPlaylist(simpleListItem.groupIndex, i)

                                RowLayout {
                                    id: row
                                    anchors.fill: parent

                                    anchors.leftMargin: simpleListItem.showMediaNumber ? listItem.margins : 0
                                    anchors.rightMargin: listItem.margins + root.rightMargin

                                    //spacing: Units.dp(16)

                                    Label {
                                        visible: simpleListItem.showMediaNumber
                                        style: "subheading"
                                        text: number

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

                                            onClicked: ui.model.play(simpleListItem.groupIndex, i)
                                        //}
                                    }
                                    Label {
                                        style: "subheading"
                                        text: title

                                        //horizontalAlignment: Text.Left
                                        //Layout.alignment: Qt.AlignLeft
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        style: "subheading"
                                        text: description
                                    }
                                }
                            }
                        }
                    }
                    }

                    Component {
                        id: viewType2
                    Row {
                        id: thumbnailItems
                        width: root.width - Units.dp(8)
                        height: itemHeight

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
                            Component.onCompleted: model = ui.model.getGroupItem(i);

                            delegate: MediaThumbnail {
                                visible: i < thumbnailItems.itemCount
                                width: parent.itemWidth
                                height: parent.itemHeight

                                mediaId: id
                                mediaTitle: title
                                mediaDescription: description
                                mediaDate: date
                                mediaLength: length

                                onClick: ui.model.addToPlaylist(thumbnailItems.groupIndex, i)
                                onPlayButtonClick: ui.model.play(thumbnailItems.groupIndex, i)
                            }
                        }
                    }
                    }

                    Component {
                        id: viewType0
                        Item {
//                            Component.onCompleted: {
//                                console.log(ui);
//                                console.log(ui.model);
//                                console.log(ui.model.getGroupItem(i));
//                                console.log(ui.model.getGroupItem(i).errorMessage);
//                            }

//                            property var model: ui.model.getGroupItem(i);
                            width: root.width - root.rightMargin
                            implicitWidth: width
                            height: emptyIcon.length > 0 ? Units.gu(2) : (lEmpty.height + Units.dp(32))
                            implicitHeight: height
                            Icon {
                                visible: emptyIcon.length > 0
                                width: height
                                size: height
                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                    top: parent.top
                                    topMargin: Units.dp(16)
                                    bottom: lEmpty.top
                                    bottomMargin: Units.dp(8)
                                }
                                name: emptyIcon
                            }
                            Label {
                                id: lEmpty
                                anchors {
                                    left: parent.left
                                    leftMargin: Units.dp(16)
                                    right: parent.right
                                    rightMargin: Units.dp(16)
                                    bottom: parent.bottom
                                    bottomMargin: Units.dp(16)
                                }
                                style: "dialog"
                                text: emptyMessage
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.Wrap
                                opacity: 0.7
                            }
                        }
                    }
                }
            }
        }
    }
}
