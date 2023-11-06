// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.spacebar

Kirigami.Page {
    id: page
    padding: 0

    property string filePath
    property string type
    property real defaultSize: Math.min(page.width, page.height)

    Component.onCompleted: applicationWindow().controlsVisible = false

    Component.onDestruction: applicationWindow().controlsVisible = true

    Rectangle {
        id: background
        anchors.fill: parent
        color: "black"

        Controls.Button {
            visible: applicationWindow().controlsVisible == false
            text: i18n("Back")
            icon.name: "go-previous"
            onClicked: pageStack.layers.pop()
            z: root.z + 1;
        }

        Flickable {
            id: flickable
            anchors.fill: parent
            contentWidth: Math.max(page.width, image.width * image.scale)
            contentHeight: Math.max(page.height, image.height * image.scale)
            interactive: !pinch.active

            Image {
                id: image
                anchors.centerIn: parent
                source: filePath
                cache: false
                fillMode: Image.PreserveAspectFit
                mipmap: true
                scale: defaultSize / Math.max(sourceSize.width, sourceSize.height, 100)

                AnimatedImage {
                    source: type == "image/gif" ? parent.source : ""
                    anchors.fill: parent
                    cache: false
                }
            }

            //TODO adjust flickable position relative to touch position
            PinchHandler {
                id: pinch
                target: image
                minimumScale: 0.1
                maximumScale: 10
                minimumRotation: 0
                maximumRotation: 0
            }

            MouseArea {
                id: dragArea
                hoverEnabled: true
                anchors.fill: parent
                onWheel: {
                    if (image.width * image.scale < 100 && wheel.angleDelta.y < 0) {
                        return
                    }
                    // minimum size
                    image.scale += image.scale * wheel.angleDelta.y / 120 / 10;

                    // position image relative to cursor
                    if (image.width * image.scale > page.width) {
                        flickable.contentX += mouseX * wheel.angleDelta.y / 120 / 10;
                    } else {
                        flickable.contentX = 0
                    }
                    if (image.height * image.scale > page.height) {
                        flickable.contentY += mouseY * wheel.angleDelta.y / 120 / 10;
                    } else {
                        flickable.contentY = 0
                    }
                }
            }
        }
    }
}
