import QtQuick 2.5

import Material 0.2
import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

NavigatorPage {
    id: root

//    title: ui ? ui.name : ""

    property MediaListUiProvider ui;
    property var columnList: [];
    property int padding: Units.dp(8)

    Rectangle {
        visible: uiState ? uiState.videoBehind : false
        anchors.fill: parent
        color: "#66ffffff"
    }

    ListView {
        id: listView

        anchors {
            left: parent.left
            right: parent.right
            //top: columnsRow.bottom
            top: parent.top
            topMargin: toolbar ? toolbar.height : 0
            bottom: parent.bottom
        }

        bottomMargin: uiState && uiState.videoBehind && uiState.osdOpen ? uiState.bottomBarHeight : 0

        headerPositioning: ListView.PullBackHeader
        header: View {
            id: columnsRow
            z: 2

            anchors {
//                top: parent.top
//                topMargin: toolbar ? toolbar.height : 0
                left: parent.left
                right: parent.right
                rightMargin: root.rightMargin
            }
            height: Units.dp(48)
            elevation: listView.atYBeginning ? 0 : 1
            Behavior on elevation {
                SequentialAnimation {
                    PauseAnimation { duration: elevation == 0 ? 200 : 0 }
                    NumberAnimation { duration: 200 }
                }
            }
            backgroundColor: listView.atYBeginning ? "#00ffffff" : "#ffffffff"
//            Behavior on backgroundColor {
//                SequentialAnimation {
//                    PauseAnimation { duration: backgroundColor == "#ffffffff" ? 0 : 200 }
//                    ColorAnimation { duration: 200 }
//                }
//            }

            Row {
                id: columns
                anchors.fill: parent

                Repeater {
                    model: ui ? ui.columnCount() : 0

                    onCountChanged: {
                        if(count > 0) {
                            listView.model = ui;
                        }
                    }

                    delegate: Item {
                        id: column

                        Component.onCompleted: {
                            root.columnList[index] = column;
                        }

                        width: implicitWidth
                        implicitWidth: { //label.implicitWidth + root.padding*2 + separator.width*2
                            //console.log("Loading implicit width for", index);
                            var result;
                            var w = ui.headerData(index, 0, 1);
                            if(w > 0) result = Units.dp(w);
                            else if(w < 0) {
                                w = 3+w;
                                var w2;
                                var len = ui.columnCount();
                                var dynamicWidths = w;
                                var space = columns.width
                                for(var i = 0; i < len; i++) {
                                    if(i != index) {
                                        if((w2 = ui.headerData(i, 0, 1)) >= 0) {
                                            space -= (w2 == 0 ? label.implicitWidth : Units.dp(w2)) + root.padding*2 + separator.width*2;
                                        } else {
                                            dynamicWidths += 3+w2;
                                        }
                                    }
                                }
                                result = space / dynamicWidths / (3-w);
                            } else {
                                result = label.implicitWidth + root.padding*2 + separator.width*2;
                            }

                            return result + root.padding*2 + separator.width*2;
                        }
                        height: columns.height
                        Label {
                            id: label
                            anchors {
                                left: parent.left
                                right: parent.right
                                leftMargin: root.padding
                                rightMargin: root.padding
                                verticalCenter: parent.verticalCenter
                            }
                            text: ui.headerData(index, 0, 0)
                            style: "caption"
                            font.weight: Font.DemiBold
                        }

                        Ink {
                            id: ink
                            anchors.fill: parent
                            anchors.rightMargin: separator.width
                        }

                        Rectangle {
                            id: separator
                            opacity: ink.containsMouse
                            Behavior on opacity {
                                NumberAnimation { duration: 200 }
                            }
                            color: "#22000000"
                            width: Units.dp(1)
                            height: Units.dp(32)
                            anchors.left: label.right
                            anchors.leftMargin: root.padding - width
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
        }

        delegate: View {
            property bool selected;

            width: listView.width
            height: Units.dp(48)

            tintColor: selected
                       ? Qt.rgba(0,0,0,0.05)
                       : ink.containsMouse ? Qt.rgba(0,0,0,0.03) : Qt.rgba(0,0,0,0)

            Ink {
                id: ink
                anchors.fill: parent
                onClicked: {
                    context.playlist.addMedia(ui.getMediaContentId(index));
                }
            }

            Row {
                height: Units.dp(48)

                Repeater {
                    model: root.columnList.length

                    delegate: Item {
                        width: implicitWidth
                        implicitWidth: root.columnList[index].implicitWidth
                        height: Units.dp(48)
                        Label {
                            anchors {
                                left: parent.left
                                right: parent.right
                                leftMargin: root.padding
                                rightMargin: root.padding
                                verticalCenter: parent.verticalCenter
                            }
                            text: ui.value(i, index); // ui.headerData(index, 0, 0)
                            style: "subheading"
                        }
                    }
                }
            }
        }
    }
}
