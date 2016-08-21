import QtQuick 2.5
import Material 0.2
import Material.Extras 0.1

View {
    id: sidePanel
	
	property bool leftSide: true
	property bool osdMode: false
    property bool userOpened: false

    property bool open: true
    property alias wasOpen: mouse.wasOpen
	
    property alias title: _toolbar.title
    property alias maxActionCount: _toolbar.maxActionCount
    property alias backAction: _toolbar.backAction
    property alias actions: _toolbar.actions

    property alias toolbar: _toolbar

    backgroundColor: osdMode ? "#88ffffff" : "#ffffffff"

	anchors.left: leftSide ? parent.left : undefined
	anchors.right: leftSide ? undefined : parent.right
	anchors.top: parent.top
	//anchors.bottom: videoBehind ? parent.bottom : barControls.top

    Behavior on anchors.leftMargin {
        NumberAnimation {
            duration: 360
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on anchors.rightMargin {
        NumberAnimation {
            duration: 360
            easing.type: Easing.InOutQuad
        }
    }

	width: Units.gridUnit * 5

	//clip: true

    anchors.leftMargin: leftSide ? (mouse.panelOpen ? 0 : -width) : 0;
    anchors.rightMargin: leftSide ? 0 : (mouse.panelOpen ? 0 : -width);

    elevation: osdMode && (leftSide && anchors.leftMargin != -width || !leftSide && anchors.rightMargin != -width) ? 3 : 0
	z: osdMode ? 5 : -5
	Behavior on elevation {
		NumberAnimation {
			duration: 280
			easing.type: Easing.InOutQuad
		}
    }

    MouseArea {
        id: mouse

        property bool panelOpen: true
        property bool wasOpen: true

        propagateComposedEvents: true
        hoverEnabled: true

        anchors.fill: parent

        onPressed: mouse.accepted = false;
        onDoubleClicked: mouse.accepted = false;

        onHoveredChanged: {
            if(open == false && mouse.panelOpen === true && containsMouse == false) mouse.panelOpen = false;
        }

        //z: 100
    }

    ActualToolbar {
        id: _toolbar

        z: 200
    }

    onOpenChanged: {
        if(open) {
            mouse.panelOpen = true;
        } else {
            if(!mouse.containsMouse) mouse.panelOpen = false;
        }
    }

    onOsdModeChanged: {
        if(osdMode && !userOpened) {
            mouse.wasOpen = open;
            open = false;
        } else {
            if(!open) open = mouse.wasOpen;
        }
    }
}
