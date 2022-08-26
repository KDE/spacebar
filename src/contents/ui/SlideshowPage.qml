// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.XmlListModel 2.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.Page {
    id: page
    padding: Kirigami.Units.largeSpacing
    title: recipients

    property real pointSize: Kirigami.Theme.defaultFont.pointSize + SettingsManager.messageFontSize
    property string recipients: ""
    property var attachments: []
    property string folder: ""
    property string smil: ""
    property int pageIndex: 1
    property int pageDuration: 500
    property var parImage: []
    property var parText: []
    property bool showControls: false

    XmlListModel {
        id: regions
        xml: smil
        query: "/smil/head/layout/region"

        XmlRole { name: "rId"; query: "@id/string()"}
        XmlRole { name: "rWidth"; query: "@width/string()" }
        XmlRole { name: "rHeight"; query: "@height/string()" }
        XmlRole { name: "rLeft"; query: "@left/string()" }
        XmlRole { name: "rTop"; query: "@top/string()" }
        XmlRole { name: "rFit"; query: "@fit/string()" }
    }

    XmlListModel {
        id: pars
        xml: smil
        query: "/smil/body/par"

        XmlRole { name: "dur"; query: "@dur/string()"}
    }

    XmlListModel {
        id: images
        xml: smil
        query: pars.query + "[" + pageIndex + "]/img"

        XmlRole { name: "src"; query: "@src/string()"}
        XmlRole { name: "region"; query: "@region/string()"}
    }

    XmlListModel {
        id: texts
        xml: smil
        query: pars.query + "[" + pageIndex + "]/text"

        XmlRole { name: "src"; query: "@src/string()"}
        XmlRole { name: "region"; query: "@region/string()"}
    }

    Timer {
        id: timer
        interval: pageDuration
        repeat: false
        onTriggered: {
            parImage = []
            for (let i = 0; i < images.count; i++) {
                const item = attachments.find(o => o.name == images.get(i).src)
                if (item) {
                    item.region = images.get(i).region
                    parImage.push(item)
                }
            }
            parImage = parImage

            parText = []
            for (let i = 0; i < texts.count; i++) {
                const item = attachments.find(o => o.name == texts.get(i).src)
                if (item) {
                    item.region = texts.get(i).region
                    parText.push(item)
                }
            }
            parText = parText

            pageDuration = parseInt(pars.get(pageIndex - 1).dur || 5000)
            pageIndex++

            if (pageIndex > pars.count) {
                pageIndex = 0
                return
            }

            timer.restart()
        }
    }

    Timer {
        id: controlsTimout
        interval: 3000
        repeat: false
        onTriggered: showControls = false
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            if (showControls) {
                showControls = false
            } else {
                showControls = true
                controlsTimout.restart()
            }
        }
    }

    Component.onCompleted: {
        timer.restart()
    }

    Repeater {
        anchors.fill:parent
        model: regions

        Column {
            id: column
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: parseInt(parent.width * parseInt(rLeft) / 100)
            anchors.topMargin: parseInt(parent.height * parseInt(rTop) / 100)
            width: parent.width * parseInt(rWidth) / 100
            height: parent.height * parseInt(rHeight) / 100
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: parImage.filter(o => o.region == rId)
                Image {
                    id: image
                    anchors.horizontalCenter: column.horizontalCenter
                    source: "file://" + folder + "/" + modelData.fileName
                    sourceSize.width: column.width
                    height: Math.min(column.height, image.implicitHeight)
                    cache: false
                    fillMode: Image.PreserveAspectFit

                    AnimatedImage {
                        source: modelData.fileName.slice(-4) == ".gif" ? parent.source : ""
                        anchors.fill: parent
                        cache: false
                    }
                }
            }

            Repeater {
                model: parText.filter(o => o.region == rId)

                Controls.Label {
                    visible: !!modelData.text
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(page.width, implicitWidth)
                    text: Utils.textToHtml(modelData.text)
                    wrapMode: Text.Wrap
                    textFormat: Text.StyledText
                    linkColor: Kirigami.Theme.linkColor
                    color: Kirigami.Theme.textColor
                    font.pointSize: pointSize
                    font.family: "Noto Sans, Noto Color Emoji"
                }
            }
        }
    }

    Controls.ToolButton {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        visible: showControls
        icon.name: timer.running ? "media-playback-pause" : "media-playback-start"
        onClicked: {
            if (timer.running) {
                timer.stop()
            } else {
                if (pageIndex == 0) {
                    pageDuration = 100
                    pageIndex = 1
                    timer.restart()
                } else {
                    timer.start()
                }
            }
        }
        z: root.z + 1;
    }
}
