// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.spacebar

Kirigami.ScrollablePage {
    id: page
    padding: Kirigami.Units.largeSpacing
    title: recipients

    property real pointSize: Kirigami.Theme.defaultFont.pointSize + SettingsManager.messageFontSize
    property string recipients: ""
    property string text: ""
    property var attachments: []
    property string folder: ""

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // message contents
        Controls.Label {
            visible: !!page.text
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: page.width - Kirigami.Units.largeSpacing * 2
            text: Utils.textToHtml(page.text)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
            linkColor: Kirigami.Theme.linkColor
            color: Kirigami.Theme.textColor
            font.pointSize: pointSize
            font.family: "Noto Sans, Noto Color Emoji"
        }

        Repeater {
            model: attachments

            Column {
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.smallSpacing

                property bool isImage: modelData.mimeType.indexOf("image/") >= 0
                property string filePath: "file://" + folder + "/" + (modelData.fileName || "")

                RowLayout {
                    visible: !isImage && !modelData.text
                    Kirigami.Icon {
                        source: modelData.iconName
                    }
                    Text {
                        text: modelData.name
                        color: Kirigami.Theme.textColor
                        font.pointSize: pointSize
                    }
                    MouseArea {
                        width: parent.width
                        height: parent.height
                        onDoubleClicked: Qt.openUrlExternally(filePath)
                    }
                }

                Image {
                    id: image
                    source: isImage ? filePath : ""
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: Math.round(page.width) - Kirigami.Units.largeSpacing * 2
                    height: Math.min(root.height * 0.8, image.implicitHeight)
                    cache: false

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (!page.flicking) {
                                pageStack.layers.push("qrc:/PreviewPage.qml", {
                                filePath: filePath,
                                type: modelData.mimeType
                            } )
                        }
                    }

                    AnimatedImage {
                        source: modelData.mimeType == "image/gif" ? parent.source : ""
                        anchors.fill: parent
                        cache: false
                    }
                }

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
}
