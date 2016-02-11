/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
    Copyright (C) 2012 David Edmundson <kde@davidedmundson.co.uk>
    Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

import QtQuick 2.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.telepathy 0.1

Item {
    width: view.width
    height: mainRect.height

    property bool isIncoming: model.type == MessagesModel.MessageTypeIncoming
    property bool isTopPart: model.previousMessageType != model.type && model.nextMessageType == model.type
    property bool isMiddlePart: model.previousMessageType == model.type && model.nextMessageType == model.type
    property bool isBottomPart: model.previousMessageType == model.type && model.nextMessageType != model.type
    property bool isSinglePart: !isTopPart && !isMiddlePart && !isBottomPart

    Rectangle {
        id: mainRect

        width: parent.width - units.gridUnit * 5
        height: body.paintedHeight + units.gridUnit

        anchors {
            left: isIncoming ? parent.left : undefined
            right: isIncoming ? undefined : parent.right
            leftMargin: isIncoming ? units.smallSpacing : 0
            rightMargin: isIncoming ? 0 : units.smallSpacing
        }

        radius: isMiddlePart ? 0 : units.gridUnit
        color: isIncoming ? Qt.rgba(0.7, 0.85, 0.92, 1) : Qt.rgba(1, 0.6, 0.25, 1)

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: isTopPart ? parent.bottom : undefined
                top: isBottomPart ? parent.top : undefined
            }
            color: parent.color
//             width: parent.width
            height: parent.height / 2
            visible: !isSinglePart
        }

        Rectangle {
            anchors {
                left: isIncoming ? parent.left : undefined
                right: isIncoming ? undefined : parent.right
                top: parent.top
            }
            color: parent.color
            width: units.gridUnit
            height: units.gridUnit
            visible: isTopPart || isSinglePart
        }

        TextEdit {
            id: body
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: units.gridUnit
                rightMargin: units.gridUnit

                verticalCenter: isMiddlePart || isSinglePart ? parent.verticalCenter : undefined

                bottom: isTopPart ? parent.bottom : undefined
                topMargin: isTopPart ? units.gridUnit * 2 : undefined

                top: isBottomPart ? parent.top : undefined
                bottomMargin: isBottomPart ? units.gridUnit * 2 : undefined
            }

            height: paintedHeight

            text: model.text
            textFormat: TextEdit.RichText
            wrapMode: TextEdit.WordWrap
            selectByMouse: true
            readOnly: true

            onLinkActivated: {
                console.log("opening link: " + link);
                Qt.openUrlExternally(link);
            }
        }
    }
}
