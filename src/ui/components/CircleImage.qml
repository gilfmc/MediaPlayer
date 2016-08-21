/*
 * QML Extras - Extra types and utilities to make QML even more awesome
 *
 * Copyright (C) 2014 Michael Spencer <sonrisesoftware@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.5
import QtGraphicalEffects 1.0

import Material 0.2

Item {
    id: item

    property alias source: image.source
    property alias status: image.status
    property alias sourceSize: image.sourceSize
    property alias asynchronous: image.asynchronous
    property alias cache: image.cache
    property alias fillMode: image.fillMode

    property alias elevation: base.elevation

    width: image.implicitWidth
    height: image.implicitHeight

    View {
        id: base
        anchors.fill: parent
        anchors.margins: -Units.dp(2)
        radius: width/2
    }

    Image {
        id: image
        anchors.fill: parent
        smooth: true
        visible: false
        mipmap: true
    }

    Image {
        id: mask
        source: Qt.resolvedUrl("circle.png")
        anchors.fill: image
        smooth: true
        visible: false
        mipmap: true
    }

    OpacityMask {
        anchors.fill: image
        source: image
        maskSource: mask
    }
}
