import QtQuick 2.5
import Material 0.2

ProgressBar {
    id: progress

    property bool seekable: true
    property bool showTooltip: false

    property string revertPositionShortcut
    //property alias oldPosition: oldCursor.position

    height: (mouseArea.containsMouse || oldCursorMouseArea.containsMouse) && seekable ? Units.dp(6) : Units.dp(4)
    anchors.topMargin: (mouseArea.containsMouse || oldCursorMouseArea.containsMouse) && seekable ? -Units.dp(1) : 0

    signal seeked(int position)

    Behavior on height {
        NumberAnimation { duration: 240 }
    }
    Behavior on anchors.topMargin {
        NumberAnimation { duration: 240 }
    }

    MouseArea {
        id: mouseArea
        enabled: seekable

        anchors {
            topMargin: -Units.dp(8)
            bottomMargin: anchors.topMargin
            fill: parent
        }

        drag.target: cursor
        drag.axis: Drag.XAxis
        drag.minimumX: -cursor.width / 2
        drag.maximumX: progress.width - cursor.radius
        drag.smoothed: false

        drag.onActiveChanged: {
            if(drag.active) {
                oldCursor.showing = true;
            } else {
                cursor.notifySeek(cursor.x);
            }
        }

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        //propagateComposedEvents: true
        //onPressed: mouse.accepted = false;
        //onDoubleClicked: mouse.accepted = false;
        onPressed: {
            showTooltip = true;
            oldCursor.showing = true;
            oldCursor.position = value;
            cursor.temporaryPosition = mouse.x - cursor.radius;
            //mouse.accepted = false;
            //oldCursor.showing = true;
        }

        onClicked: {
            //cursor.x = mouse.x - cursor.radius;
            //console.log("x: ", mouse.x);
            oldCursor.showing = true;
            cursor.notifySeek(mouse.x - cursor.radius);
        }
    }

    View {
        id: oldCursor

        visible: seekable // width > 1
        //scale: mouseArea.containsMouse ? 1 : 0

        width: Units.dp(16)
        height: width
        radius: width / 2

        property bool showing: false
        property int position: -1

        elevation: 1
        opacity: showing ? 1 : 0

        scale: oldCursorMouseArea.containsMouse || oldCursor.opacity > 0 ? 1 : 0

        backgroundColor: '#aaaaaa' // Theme.accentColor

        anchors {
            left: progress.left
            leftMargin: showing ? progress.value * progress.width / progress.maximumValue - radius : position >= 0 ? position * progress.width / progress.maximumValue - radius : 0
            verticalCenter: parent.verticalCenter
        }

        Behavior on opacity {
            NumberAnimation {
                id: oldCursorAnimation
            }
        }

        onShowingChanged: {
            if(showing) oldCursorAnimation.duration = 0;
            else oldCursorAnimation.duration = 3000;
        }

        MouseArea {
            id: oldCursorMouseArea
            enabled: seekable
            anchors.fill: parent

            //enabled: containsMouse || oldCursor.opacity > 0

            hoverEnabled: true
            propagateComposedEvents: true

            onPressed: {
                tooltip.stopTooltip();
                mouse.accepted = false;
            }
        }
    }

    View {
        id: cursor

        visible: seekable // width > 1
        scale: mouseArea.containsMouse || mouseArea.drag.active ? 1 : 0

        width: Units.dp(16)
        height: width
        radius: width / 2

        property int temporaryPosition: -1

        elevation: 1

        backgroundColor: Theme.accentColor

        anchors {
            left: /*dragger.drag.active ||*/ mouseArea.drag.active ? undefined : progress.left
            leftMargin: temporaryPosition >= 0 ? temporaryPosition : progress.value * progress.width / progress.maximumValue - radius
            verticalCenter: parent.verticalCenter
        }

        Behavior on scale {
            NumberAnimation { duration: 180 }
        }

//        MouseArea {
//            id: dragger
//            anchors.fill: parent
//            anchors.margins: -Units.dp(4)

//            acceptedButtons: "LeftButton"

//            drag.target: cursor
//            drag.axis: Drag.XAxis
//            drag.minimumX: -cursor.width / 2
//            drag.maximumX: progress.width - cursor.radius
//            drag.smoothed: false

//            drag.onActiveChanged: {
//                if(!drag.active) {
//                    cursor.notifySeek(cursor.x);
//                }
//            }

////            drag.onDragFinished: {
////                var position = (dropAction.x + cursor.radius) / progress.width * progress.maximumValue;
////                onSeeked(position)
////            }
//        }

        function notifySeek(x) {
            var position = (x + cursor.radius) / progress.width * progress.maximumValue;
            seek(position);
        }
    }

    function seek(pos) {
        cursor.temporaryPosition = -1;
        oldCursor.position = value;
        seeked(pos);
        showTooltip = false;
        oldCursor.showing = false;
    }

    Item {
        id: tooltip
//        implicitHeight: dragger.drag.active || progress.focus
//                ? Units.dp(48) : 0
//        implicitWidth: implicitHeight
        width: tt.width
        height: tt.height // Units.dp(32)

        property bool normalMode: /*dragger.drag.active ||*/ showTooltip || progress.focus
        property bool oldPositionMode: oldCursorMouseArea.containsMouse

        onNormalModeChanged: {
            if(normalMode) {
                anchors.horizontalCenter = cursor.horizontalCenter;
                tt.backgroundColor = Theme.accentColor;
                tt.opacity = 1;
            } else if(!oldPositionMode) {
                tt.opacity = 0;
            }
        }
        onOldPositionModeChanged: {
            if(oldPositionMode) {
                anchors.horizontalCenter = oldCursor.horizontalCenter;
                tt.backgroundColor = Theme.dark.hintColor;
                tt.opacity = 1;
            } else if(!normalMode) {
                tt.opacity = 0;
            }
        }

        function stopTooltip() {
            anchors.horizontalCenter = undefined;
            tt.opacity = 0;
        }

        anchors {
            bottom: cursor.top
            //horizontalCenter: oldCursorMouseArea.containsMouse ? oldCursor.horizontalCenter : (dragger.drag.active || showTooltip || progress.focus ? cursor.horizontalCenter : undefined)
            bottomMargin: Units.dp(16)
        }

        Card {
            id: tt
            width: time.width + time.anchors.leftMargin + time.anchors.rightMargin
            height: time.height + time.anchors.topMargin + time.anchors.bottomMargin

            anchors {
                top: parent.top
                //topMargin: seekable ? (dragger.drag.active || showTooltip || oldCursorMouseArea.containsMouse || progress.focus ? 0 : Units.dp(8)) : 0
                //bottomMargin: Units.dp(8)
            }

            //backgroundColor: oldCursorMouseArea.containsMouse ? Theme.dark.hintColor : Theme.accentColor
            elevation: 1

            Label {
                id: time

                property bool showHours: maximumValue >= 3600000

                anchors {
                    centerIn: parent
                    topMargin: Units.dp(4)
                    bottomMargin: Units.dp(4)
                    leftMargin: Units.dp(8)
                    rightMargin: Units.dp(8)
                }

                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                function formatTime(timeToFormat) {
                    var date = new Date(timeToFormat);
                    var zeroes = "00";
                    if(time.showHours) return "%1:%2:%3".arg(date.getUTCHours()).arg((zeroes+date.getUTCMinutes()).slice(-zeroes.length)).arg((zeroes+date.getUTCSeconds()).slice(-zeroes.length));
                    else return "%1:%2".arg((zeroes+date.getUTCMinutes()).slice(-zeroes.length)).arg((zeroes+date.getUTCSeconds()).slice(-zeroes.length));
                }

                Binding {
                    target: time
                    property: 'text'
                    value: {
                        return time.formatTime((cursor.x + cursor.radius) / progress.width * progress.maximumValue);
                    }
                    when: tooltip.normalMode
                }
                Binding {
                    target: time
                    property: 'text'
                    value: {
                        if(revertPositionShortcut) return time.formatTime(oldCursor.position) + " <i>" + revertPositionShortcut + "</i>";
                        else return time.formatTime(oldCursor.position);
                    }
                    when: tooltip.oldPositionMode && tooltip.horizontalCenter !== undefined
                }
                style: "title"
                fontSizeMode: Text.Fit
                font.pixelSize: parent.implicitHeight - Units.dp(19)
                minimumPixelSize: Units.dp(6)
                //wrapMode: Text.WordWrap
                color: Theme.lightDark(styleColor,
                                        Theme.light.textColor,
                                        Theme.dark.textColor)
                z: 1

                property color styleColor: /*control.hasOwnProperty("color")
                        ? control.color : */Theme.light.accentColor
            }

            opacity: 0 // seekable ? (dragger.drag.active || showTooltip || oldCursorMouseArea.containsMouse || progress.focus ? 1 : 0) : 0
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
            Behavior on anchors.topMargin {
                NumberAnimation { duration: 200 }
            }
        }

//        Rectangle {
//            id: roundKnob
//            //anchors.fill: parent
//            anchors {
//                horizontalCenter: parent.horizontalCenter
//                verticalCenter: parent.verticalCenter
//            }

//            implicitHeight: parent.height
//            implicitWidth: parent.width
//            radius: implicitWidth / 2
//            color: Theme.accentColor
//            antialiasing: true
//            clip: true

//            Rectangle {
//                implicitHeight: parent.height / 2
//                implicitWidth: parent.width / 2
//                color: Theme.accentColor
//                anchors.right: parent.right
//                anchors.bottom: parent.bottom
//                antialiasing: true
//            }

//            transform: [
//                Rotation {
//                    //origin { x: implicitWidth/10; y: implicitHeight/10 }
//                    angle: dragger.drag.active || progress.focus ? 45 : 0
//                    Behavior on angle {
//                        NumberAnimation { duration: 1000 }
//                    }
//                }
//            ]
//        }

        Behavior on implicitHeight {
            NumberAnimation { duration: 20}
        }

        Behavior on implicitWidth {
            NumberAnimation { duration: 20}
        }
    }

    function revertPosition() {
        var oldPosition = oldCursor.position;
        if (oldPosition != -1) {
            seek(oldCursor.position);
        }
    }
}
