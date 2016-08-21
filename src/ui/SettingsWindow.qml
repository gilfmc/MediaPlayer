import QtQuick 2.5
import QtQuick.Layouts 1.1

import Material 0.2
import Material.ListItems 0.1 as ListItems

ApplicationWindow {
    id: window

    property string page;
    property string _currentPage;

    title: qsTr("Settings")

    width: Units.gu(10)
    height: Units.gu(7)

//    theme {
//        primaryColor: Palette.colors.orange["500"]
//        primaryDarkColor: Palette.colors.orange["700"]
//        accentColor: Palette.colors.orange["A400"]
//    }

    ListModel {
        id: rootOptions

        ListElement {
            pageId: "general"
            title: qsTr("General")
            icon: "action/settings_applications"
            page: "SettingsGeneralPage.qml"
        }
        ListElement {
            pageId: "appearance"
            title: qsTr("Appearance")
            icon: "image/color_lens"
            page: "SettingsAppearancePage.qml"
        }
        ListElement {
            pageId: "library"
            title: qsTr("Library")
            icon: "av/subscriptions"
            page: "SettingsLibraryPage.qml"
        }
        ListElement {
            pageId: "extensions"
            title: qsTr("Extensions")
            icon: "action/extension"
            page: "SettingsExtensionsPage.qml"
        }
        ListElement {
            pageId: "about"
            title: qsTr("About")
            icon: "action/info"
            page: "SettingsAboutPage.qml"
        }
    }

    Sidebar {
        id: sidebar

        expanded: pageStack.depth > 1 && parent.width > Units.gu(8)
        anchors.top: pageStack.top
        autoFlick: false

        z: -1

        ListView {
            anchors.fill: parent

            model: rootOptions
            delegate: ListItems.Standard {
                text: title
                selected: page == _currentPage
                iconName: icon
                interactive: !selected
                onClicked: _openPage(page)
            }
        }
    }

    Object {
        id: uiState
        readonly property int leftKeyline: sidebar.expanded ? (Units.dp(24) + sidebar.width) : (Units.dp(72) + 0)
        function showSnackbar(text, time) {
            errorToast.open(text);
        }
    }

    initialPage: Page {
        title: qsTr("Settings")

        GridView {
            id: grid

            anchors.fill: parent
            anchors.margins: Units.dp(16)

            model: rootOptions

            property int columnCount: Math.max(1, Math.round(width / Units.dp(192)))

            cellWidth: width / columnCount
            cellHeight: Units.dp(160)

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                Ink {
                    id: ink
                    onClicked: _openPage(page)
                    anchors.fill: parent
                    z: -1
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Units.dp(8)
                    spacing: Units.dp(8)

                    Icon {
                        anchors.horizontalCenter: parent.horizontalCenter

                        color: Theme.light.iconColor
                        size: Units.dp(64)

                        name: icon
                    }
                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter

                        elide: Text.ElideRight
                        style: "subheading"

                        color: Theme.light.textColor

                        text: title
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        //Units.multiplier = 1;
        //Units.pixelDensity /= 1.4;
        Units.multiplier = context.settings.dpMultiplier(Units.multiplier);

        var previousPage = null;

        pageStack.z = -2
        pageStack.anchors.left = sidebar.right

//        pageStack.popped.connect(function(page) {
//            console.log(page.title, "was popped");
//            if (typeof page.save === "function") page.save();
//            page.destroy(1000);
//        });

        pageStack.onCurrentItemChanged.connect(function() {
            if (previousPage && typeof previousPage.save === "function") previousPage.save();
            previousPage = pageStack.currentItem;
        });

        if(page) openPage(page);
    }

    onClosing: {
        var len = pageStack.depth;
        var i;
        for(i = 0; i < len; i++) {
            var page = pageStack.get(i, true);
            if(page && typeof page.save === "function") page.save();
        }
        for(i = 0; i < len; i++) {
            pageStack.pop();
        }
    }

    Snackbar {
        id: errorToast
    }

    function openPage(page) {
        var len = rootOptions.count;
        for(var i = 0; i < len; i++) {
            if(rootOptions.get(i).pageId === page) {
                _openPage(rootOptions.get(i).page);
                break;
            }
        }
    }

    function _openPage(page) {
        // TODO maybe pages should be cached (lazy-loaded)?
        var component = Qt.createComponent(page);
        if (component && component.status === Component.Ready) {
            if(pageStack.depth > 1) {
                while(pageStack.depth > 2) {
                    var oldPage = pageStack.pop();
                    //if (typeof oldPage.save === "function") oldPage.save();
                    oldPage.destroy(1000);
                }
                pageStack.push({item:component.createObject(window, {"uiState": uiState}), replace: true, destroyOnPop:true});
            } else {
                pageStack.push({item:component.createObject(window, {"uiState": uiState}), destroyOnPop:true});
            }
            _currentPage = page;
        } else {
            console.log("Error loading settings page:", component.errorString());
            errorToast.open(qsTr("This option failed to load."));
        }
    }
}
