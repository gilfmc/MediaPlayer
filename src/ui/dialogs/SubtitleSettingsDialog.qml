import QtQuick 2.5
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0

import Material 0.2
import Material.ListItems 0.1 as ListItems

import org.papyros.mediaplayer 1.0

import "../components"

Dropdown {
    id: subtitleSettingsDialog

    property alias subtitlesEnabled: switchSubtitles.checked
    property alias subtitleSize: sSubtitleSize.value
    property alias subtitleDelay: sDelay.value

    property string filePickerFolder: "~"
    signal onSubtitleFileOpened(string file)
	
    width: Units.gu(5)
    height: bOpen.y + bOpen.height + Units.dp(16)

    View {
        id: toolbar

        height: Units.gridUnit - Units.dp(8)
        anchors {
            //top: parent.top
            left: parent.left
            right: parent.right
            rightMargin: -1
        }
        backgroundColor: Theme.primaryColor

        Ink {
            z: 0
            anchors.fill: parent
            anchors.rightMargin: parent.height
            onClicked: switchSubtitles.checked = !switchSubtitles.checked
        }
        Switch {
            id: switchSubtitles
            z: 1
            color: "#f0f0f0"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: Units.dp(16)
        }
        Label {
            anchors.left: switchSubtitles.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: Units.dp(12)

            text: aSubtitles.name
            style: "title"
            color: Theme.dark.textColor
        }
        IconButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: Units.dp(16)

            action: Action {
                name: qsTr("Close")
                iconName: "navigation/close"
                onTriggered: subtitleSettingsDialog.close();
            }
            color: Theme.dark.iconColor
        }
    }

    Label {
        id: lSize
        anchors {
            left: parent.left
            right: parent.right
            top: toolbar.bottom
            margins: Units.dp(16)
            topMargin: Units.dp(8)
        }
        text: qsTr("Size")
        style: "subheading"
    }
    Slider {
        id: sSubtitleSize
        anchors {
            left: parent.left
            right: parent.right
            top: lSize.bottom
            margins: Units.dp(16)
            topMargin: 0
        }

        minimumValue: 24
        maximumValue: 80
    }

    Label {
        id: lDelay
        anchors {
            left: parent.left
            right: parent.right
            top: sSubtitleSize.bottom
            margins: Units.dp(16)
            topMargin: Units.dp(8)
        }
        text: qsTr("Delay")
        style: "subheading"
    }
    Slider {
        id: sDelay
        anchors {
            left: parent.left
            right: bResetDelay.left
            top: lDelay.bottom
            margins: Units.dp(16)
            topMargin: -Units.dp(32)
        }

        numericValueLabel: true
        knobDiameter: Units.dp(52)
        knobLabel: value > 0 ? ('+' + (Math.round(value)/1000) + 's') : ((Math.round(value)/1000) + 's')

        minimumValue: -2500
        maximumValue: +2500
    }
    IconButton {
        id: bResetDelay
        anchors {
            right: parent.right
            top: lDelay.bottom
            margins: Units.dp(16)
            topMargin: 0
        }
        action: Action {
            name: qsTr("Reset delay")
            iconName: "action/autorenew"
            onTriggered: sDelay.value = 0;
        }
    }

    Button {
        id: bOpen
        anchors {
            left: parent.left
            right: parent.right
            top: sDelay.bottom
            margins: Units.dp(16)
        }
        text: qsTr("Open file...");
        onClicked: showFilePicker();
    }

    Loader {
        id: filePicker
        asynchronous: true

        sourceComponent: FileDialog {
            title: "Open a subtitles file"
            folder: shortcuts.home

            nameFilters: [ qsTr("SubRip subtitle file") + " (*.srt)" ]

            onFileUrlChanged: onSubtitleFileOpened(fileUrl)
        }
    }

    function showFilePicker() {
        filePicker.item.open();
    }
}
