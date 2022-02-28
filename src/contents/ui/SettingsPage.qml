// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: page
    title: i18n("Settings")
    topPadding: 0

    property ChatListModel chatListModel;

    Component.onDestruction: if (chatListModel) chatListModel.saveSettings()

    width: applicationWindow().width
    Kirigami.ColumnView.fillWidth: true
    
    Kirigami.FormLayout {
        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            onPressed: parent.forceActiveFocus()
        }

        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18n("Appearance")
        }

        Controls.CheckBox {
            id: customMessageColors
            checked: SettingsManager.customMessageColors
            text: i18n("Use custom colors for messages")
            onToggled: SettingsManager.customMessageColors = checked
        }

        Row {
            leftPadding: Kirigami.Units.gridUnit * 2
            bottomPadding: Kirigami.Units.gridUnit
            spacing: Kirigami.Units.gridUnit
            visible: SettingsManager.customMessageColors

            Row {
                spacing: Kirigami.Units.smallSpacing

                Controls.Button {
                    id: incomingMessageColor
                    width: Kirigami.Units.gridUnit * 1.5
                    height: Kirigami.Units.gridUnit * 1.5
                    enabled: SettingsManager.customMessageColors
                    onClicked: {
                        colorDialog.selected = SettingsManager.incomingMessageColor
                        colorDialog.field = "incomingMessageColor"
                        colorDialog.open()
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 1
                        color: SettingsManager.incomingMessageColor
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: i18n("Incoming")
                    color: Kirigami.Theme.textColor
                }
            }

            Row {
                spacing: Kirigami.Units.smallSpacing

                Controls.Button {
                    id: outgoingMessageColor
                    width: Kirigami.Units.gridUnit * 1.5
                    height: Kirigami.Units.gridUnit * 1.5
                    enabled: SettingsManager.customMessageColors
                    onClicked: {
                        colorDialog.selected = SettingsManager.outgoingMessageColor
                        colorDialog.field = "outgoingMessageColor"
                        colorDialog.open()
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 1
                        color: SettingsManager.outgoingMessageColor
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: i18n("Outgoing")
                    color: Kirigami.Theme.textColor
                }
            }
        }

        Controls.SpinBox {
            id: messageFontSize
            value: SettingsManager.messageFontSize
            Kirigami.FormData.label: i18n("Message font size:")
            from: -3
            to: 3
            stepSize: 1
            onValueModified: SettingsManager.messageFontSize = value
        }


        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18n("Multimedia messages (MMS)")
        }

        Controls.TextField {
            id: mmsc
            placeholderText: "http://example.com/mms/wapenc"
            text: SettingsManager.mmsc
            Kirigami.FormData.label: i18n("MMSC:")
            onTextChanged: SettingsManager.mmsc = text
            wrapMode: TextInput.WrapAnywhere
        }

        Controls.TextField {
            id: mmsProxy
            placeholderText: "proxy.example.com"
            text: SettingsManager.mmsProxy
            Kirigami.FormData.label: i18n("Proxy:")
            onTextChanged: SettingsManager.mmsProxy = text
            wrapMode: TextInput.WrapAnywhere
        }

        Controls.SpinBox {
            id: mmsPort
            value: SettingsManager.mmsPort
            Kirigami.FormData.label: i18n("Port:")
            from: 0
            to: 65535
            stepSize: 1
            onValueModified: SettingsManager.mmsPort = value
            textFromValue: function(value, locale) {
                return value
            }
        }

        Controls.CheckBox {
            id: requestDeliveryReports
            checked: SettingsManager.requestDeliveryReports
            text: i18n("Request delivery reports")
            onToggled: SettingsManager.requestDeliveryReports = checked
        }

        Controls.CheckBox {
            id: requestReadReports
            checked: SettingsManager.requestReadReports
            text: i18n("Request read reports")
            onToggled: SettingsManager.requestReadReports = checked
        }

        Controls.CheckBox {
            id: shareDeliveryStatus
            checked: SettingsManager.shareDeliveryStatus
            text: i18n("Share delivery status")
            onToggled: SettingsManager.shareDeliveryStatus = checked
        }

        Controls.CheckBox {
            id: shareReadStatus
            checked: SettingsManager.shareReadStatus
            text: i18n("Share read status")
            onToggled: SettingsManager.shareReadStatus = checked
            enabled: false
        }

        Controls.CheckBox {
            id: autoDownload
            checked: SettingsManager.autoDownload
            text: i18n("Auto download messages")
            onToggled: SettingsManager.autoDownload = checked
        }

        Row {
            leftPadding: Kirigami.Units.gridUnit * 2

            Controls.CheckBox {
                id: autoDownloadContactsOnly
                checked: SettingsManager.autoDownloadContactsOnly
                text: i18n("Existing contacts only")
                onToggled: SettingsManager.autoDownloadContactsOnly = checked
                enabled: SettingsManager.autoDownload == true
            }
        }

        Controls.SpinBox {
            id: totalMaxAttachmentSize
            value: SettingsManager.totalMaxAttachmentSize
            Kirigami.FormData.label: i18n("Max message size (KiB):")
            from: 100
            to: 5000
            stepSize: 50
            onValueModified: SettingsManager.totalMaxAttachmentSize = value
        }

        Controls.SpinBox {
            id: maxAttachments
            value: SettingsManager.maxAttachments
            Kirigami.FormData.label: i18n("Max attachments:")
            from: 1
            to: 25
            onValueModified: SettingsManager.maxAttachments = value
        }

        Controls.CheckBox {
            id: autoCreateSmil
            checked: SettingsManager.autoCreateSmil
            text: i18n("Auto create SMIL")
            onToggled: SettingsManager.autoCreateSmil = checked
        }

        Controls.CheckBox {
            id: groupConversation
            checked: SettingsManager.groupConversation
            text: i18n("Default to group conversations")
            onToggled: SettingsManager.groupConversation = checked
        }


        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18n("Notifications")
        }

        Controls.CheckBox {
            id: showSenderInfo
            checked: SettingsManager.showSenderInfo
            text: i18n("Show sender name / number")
            onToggled: SettingsManager.showSenderInfo = checked
        }

        Controls.CheckBox {
            id: showMessageContent
            checked: SettingsManager.showMessageContent
            text: i18n("Show a preview of the message content")
            onToggled: SettingsManager.showMessageContent = checked
        }


        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18n("Other")
        }

        Controls.Button {
            text: i18n("Restore defaults")
            onClicked: {
                const server = SettingsManager.mmsc
                const proxy = SettingsManager.mmsProxy
                chatListModel.restoreDefaults()
                SettingsManager.mmsc = server
                SettingsManager.mmsProxy = proxy
            }
        }
        
        Controls.Button {
            text: i18n("About")
            icon.name: "help-about-symbolic"
            onClicked: applicationWindow().pageStack.push("qrc:/AboutPage.qml")
        }
    }

    Kirigami.OverlaySheet {
        id: colorDialog
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding:0

        property string field
        property color selected

        title: i18n("Choose a color")

        Grid {
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing
            padding: Kirigami.Units.largeSpacing * 2
            flow: Grid.TopToBottom
            columns: 9

            property real cellSize: Math.min(page.width / columns - Kirigami.Units.largeSpacing, Kirigami.Units.gridUnit * 2.5)

            Repeater {
                model: [
                    "#99c1f1", "#62a0ea", "#3584e4", "#1c71d8", "#1a5fb4",
                    "#8ff0a4", "#57e389", "#33d17a", "#2ec27e", "#26a269",
                    "#fff4a2", "#f9f06b", "#f8e45c", "#f6d32d", "#f5c211",
                    "#ffbe6f", "#ffa348", "#ff7800", "#e66100", "#c64600",
                    "#f66151", "#ed333b", "#e01b24", "#c01c28", "#a51d2d",
                    "#dc8add", "#c061cb", "#9141ac", "#813d9c", "#613583",
                    "#cdab8f", "#b5835a", "#986a44", "#865e3c", "#63452c",
                    "#ffffff", "#f6f5f4", "#deddda", "#c0bfbc", "#9a9996",
                    "#77767b", "#5e5c64", "#3d3846", "#241f31", "#000000"
                ]

                delegate: Controls.Button {
                    width: parent.cellSize
                    height: parent.cellSize
                    background: Rectangle {
                        color: modelData
                        border.width: 1
                        border.color: Kirigami.Theme.disabledTextColor
                    }
                    onClicked: colorDialog.selected = modelData

                    Kirigami.Icon {
                        visible: modelData == colorDialog.selected
                        anchors.fill: parent
                        source: "object-select-symbolic"
                        color: modelData == "#000000" ? "white" : "black"
                    }
                }
            }
        }

        footer: RowLayout {
            Controls.Button {
                text: i18n("Cancel")
                onClicked: colorDialog.close()
            }

            // spacer
            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18n("Set")
                onClicked: {
                    SettingsManager[colorDialog.field] = colorDialog.selected
                    colorDialog.close()
                }
            }
        }
    }
}
