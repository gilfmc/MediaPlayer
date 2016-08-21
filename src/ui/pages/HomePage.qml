import QtQuick 2.5
import Material 0.2

import "../components"

NavigatorPage {
    id: root

    property var ui
    onUiChanged: {
        if(ui === null) return;
        var len = ui.widgetCount;
        for(var i = 0; i < len; i++) {
            var card = ui.widgetQmlItemAt(i);
            card.homeUi = homeUi;
            card.parent = mainContent;
        }
    }

    title: qsTr("Home")

//    anchors {
//        fill: parent
//        leftMargin: Units.dp(16)
//        topMargin: Units.dp(16) + toolbarMain.height
//    }

    Object {
        id: homeUi

        function openCardMenu(card, from, x, y) {
            popupMenu.open(card, from, x, y);
        }
    }

    Loader {
        id: popupMenu
        function open(card, from, x, y) {
            sourceComponent = componentPopupMenu;
            item.card = card;
            item.open(from, x, y);
        }

        Component {
            id: componentPopupMenu
            PopupMenu {
                property Item card;
                actions: [
                    Action {
                        text: qsTr("Dismiss card")
                        onTriggered: {
                            dismissOpacityAnimation.target = card;
                            card.transform = dismissTranslate;
                            dismissTranslate.dismiss(card);
                            dismissOpacityAnimation.start();
                        }
                    }
                ]
            }
        }
    }

    NumberAnimation {
        id: dismissOpacityAnimation
        property: "opacity"
        to: 0
        duration: 260
    }
    Translate {
        id: dismissTranslate

        function dismiss(card) {
            dismissAnimation.stop();
            x = 0;
            dismissAnimation.card = card;
            dismissAnimation.start();
        }
    }
    NumberAnimation {
        property Item card;
        id: dismissAnimation
        target: dismissTranslate
        property: "x"
        to: card.width
        duration: 280
        easing.type: Easing.InOutCubic
        onRunningChanged: {
            if(!running){
                if(card.model && card.model.notifyCardDismissed) card.model.notifyCardDismissed();
            }
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: mainContent.height + mainContent.anchors.topMargin + Units.dp(16)

        Behavior on contentHeight {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }

        bottomMargin: uiState && uiState.videoBehind && uiState.osdOpen ? uiState.bottomBarHeight : 0

    Column {
        id: mainContent
        width: parent.width

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: Units.dp(16)
            right: parent.right
            rightMargin: root.rightMargin + Units.dp(16)
            topMargin: Units.dp(16) + toolbarMain.height
        }

        spacing: Units.dp(16)

        move: Transition {
            NumberAnimation { property: "y"; duration: 240; easing.type: Easing.InOutCubic }
        }

//        add: Transition {
//            NumberAnimation { properties: "y"; from: root.height; duration: 400; easing.type: Easing.OutCubic }
//        }
        Row {
            width: parent.width

            spacing: Units.dp(16)
            property int itemSize: (parent.width - 2 * spacing) / 3

            Button {
                elevation: 1
                width: parent.itemSize
                text: qsTr("Music")
                backgroundColor: Theme.accentColor
                onClicked: openMediaPropertyList(0, text);
            }
            Button {
                elevation: 1
                width: parent.itemSize
                text: qsTr("TV Shows")
                backgroundColor: Theme.accentColor
                onClicked: openMediaPropertyList(1, text);
            }
            Button {
                elevation: 1
                width: parent.itemSize
                text: qsTr("Movies")
                backgroundColor: Theme.accentColor
                onClicked: uiState.showSnackbar(qsTr("Movies will be supported in a future version, you can still open movies manually though."), 6500)
            }
        }
    }
    }

    function openMediaPropertyList(number, title) {
        var page = openPage("MediaPropertyList");
        page.ui = libraryModel.getListUi(number);
        page.title = title;
    }
}
