import QtQuick 2.5
import QtQuick.Layouts 1.1

import Material 0.2

Page {
    title: qsTr("About")

    Scrollbar {
        flickableItem: flickable
    }

    Flickable {
        id: flickable
        anchors.fill: parent

        contentHeight: {
            var len = contentItem.children.length;
            var sum = 0;
            var c;
            for(var i = 0; i < len; i++) {
                c = contentItem.children[i];
                sum += c.height + c.anchors.topMargin + c.anchors.bottomMargin;
            }
            return sum;
        }

        Image {
            id: appIcon
            anchors {
                top: parent.top
                topMargin: Units.dp(16)
                horizontalCenter: parent.horizontalCenter
            }
            property int maximumHeight: Units.dp(160)
            height: implicitHeight > maximumHeight ? maximumHeight : implicitHeight
            width: height
            source: "icon.png"
        }
        Label {
            id: appName
            style: "title"
            anchors {
                top: appIcon.bottom
                topMargin: Units.dp(8)
                horizontalCenter: parent.horizontalCenter
            }
            text: app.applicationName
        }
        Label {
            id: appVersion
            style: "dialog"
            anchors {
                top: appName.bottom
                topMargin: Units.dp(8)
                horizontalCenter: parent.horizontalCenter
            }
            text: qsTr("Version %1").arg(app.applicationVersion)
        }

        Label {
            id: appDevelopers
            style: "title"
            anchors {
                top: appVersion.bottom
                topMargin: Units.dp(32)
                horizontalCenter: parent.horizontalCenter
            }
            text: qsTr("Developers")
        }
        ColumnLayout {
            id: appDevelopersList
            anchors {
                top: appDevelopers.bottom
                topMargin: Units.dp(8)
                horizontalCenter: parent.horizontalCenter
            }
            Label {
                style: "subheading"
                text: "Gil Castro"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        Label {
            id: appTranslators
            style: "title"
            anchors {
                top: appDevelopersList.bottom
                topMargin: Units.dp(32)
                horizontalCenter: parent.horizontalCenter
            }
            text: qsTr("Translators")
        }
        ColumnLayout {
            id: appTranslatorsList
            anchors {
                top: appTranslators.bottom
                topMargin: Units.dp(8)
                horizontalCenter: parent.horizontalCenter
            }
            Label {
                style: "body2"
                text: qsTr("Portuguese (Portugal)")
                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                style: "subheading"
                text: "Gil Castro"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        Label {
            anchors {
                top: appTranslatorsList.bottom
                topMargin: Units.dp(20)
                bottomMargin: Units.dp(20)
                horizontalCenter: parent.horizontalCenter
            }
            style: "caption"
            text: "© 2016 Gil Castro"
        }
    }
}
