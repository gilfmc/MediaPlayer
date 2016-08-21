import QtQuick 2.5
import Material 0.2

Page {
    actionBar.visible: false
    backgroundColor: "#00ffffff"
    //anchors.fill: parent
    anchors.rightMargin: rightMargin

    property bool bigPictureMode: false;
    property real toolbarHeight: -1;

    property Item toolbar;
    property Item navigator;

    property Item rightPanel;
    property real rightMargin: rightPanel ? rightPanel.width + rightPanel.anchors.rightMargin : 0;

    property var uiState;

    function openPage(pageName) {
        return navigator.openPage(pageName);
//        var component = Qt.createComponent("../pages/" + pageName + ".qml");
//        if (component && component.status === Component.Ready) {
//            var page = component.createObject(navigator);
//            page.navigator = navigator;
//            page.toolbar = toolbar;
//            page.rightPanel = rightPanel;
//            page.uiState = uiState;

//            navigator.push(page);
//            return page;
//        } else {
//            console.log("Couldn't load \"../pages/" + pageName + ".qml\": " + component.errorString());
//        }

//        return null;
    }
}
