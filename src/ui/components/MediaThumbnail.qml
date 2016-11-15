import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import Material 0.2

Card {
    property int mediaId
    property alias mediaTitle: titleLabel.text
    property alias mediaDescription: descriptionLabel.text
    property alias mediaDate: lDate.text
    property alias mediaLength: lLength.text

    signal click
    signal playButtonClick
	
	Image {
		anchors.fill: parent
        source: parent.visible ? "image://mp/thumb/" + mediaId : ""
        fillMode: Image.PreserveAspectCrop
	}
	
	LinearGradient {
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		
		height: titleLabel.height * 7
		
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
		
		height: bInfo.height * 3
		
		cached: true
		
		start: Qt.point(0, 0)
		end: Qt.point(0, height)
		
		gradient: Gradient {
			GradientStop { position: 0.00; color: "#66000000" } // #33
			GradientStop { position: 0.33; color: "#32000000" } // #19
			GradientStop { position: 1.00; color: "#00000000" }
		}
	}
	
	Column {
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			margins: Units.dp(4)
		}
		
		Label {
            id: titleLabel
			style: "subheading"
			font.weight: Font.DemiBold
			color: Theme.dark.textColor
		}
		Label {
            id: descriptionLabel
			visible: text.length > 0
			style: "body1"
			color: Theme.dark.textColor
		}
	}
	
	Label {
        id: lDate
		anchors {
			right: bInfo.left
			top: parent.top
			margins: Units.dp(4)
        }
		style: "caption"
		color: Theme.dark.textColor
	}
	
	Label {
        id: lLength
		anchors {
			right: parent.right
			bottom: parent.bottom
			margins: Units.dp(4)
        }
		style: "caption"
		color: Theme.dark.textColor
	}
	
	Ink {
		id: ink
		anchors.fill: parent
		hoverEnabled: true
		onClicked: {
			var x = mouse.x, y = mouse.y;
			var cx = ink.width/2, cy = ink.height/2
			var distance = Math.sqrt(Math.pow(x - cx, 2) + Math.pow(y - cy, 2));
			if(distance <= Units.dp(28))
                playButtonClick();
			else
                click();
		}
		color: Qt.rgba(1,1,1,0.1)
	}
	
	IconButton {
		id: bPlay
		anchors {
			centerIn: parent
			margins: Units.dp(16)
		}
		action: Action {
			iconName: "av/play_circle_filled"
			text: qsTr("Play")
		}
		size: Units.dp(48)
		color: "#8affffff"
		opacity: ink.containsMouse ? 1 : 0
		Behavior on opacity { NumberAnimation { duration: 160 } }
	}
	
	IconButton {
        id: bInfo
		action: Action {
			iconName: "action/info"
			text: qsTr("Info")
		}
		size: Units.dp(18)
		color: "#deffffff"
        // TODO make it visible once implemented:
		anchors {
			right: parent.right
            rightMargin: -width // ink.containsMouse ? Units.dp(4) : -width
			top: parent.top
			topMargin: Units.dp(4)
        }
        visible: false
//		opacity: ink.containsMouse ? 1 : 0
//		Behavior on opacity { NumberAnimation { duration: 140 } }
//		Behavior on anchors.rightMargin { NumberAnimation { duration: 160; easing.type: Easing.InOutCubic } }
	}
}
