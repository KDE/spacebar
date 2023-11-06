// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.spacebar
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: page
    title: i18n("Settings")

    leftPadding: 0
    rightPadding: 0
    topPadding: Kirigami.Units.gridUnit
    bottomPadding: Kirigami.Units.gridUnit

    property ChatListModel chatListModel;

    Component.onDestruction: if (chatListModel) chatListModel.saveSettings()

    width: applicationWindow().width
    Kirigami.ColumnView.fillWidth: true
    
    ColumnLayout {
        spacing: 0

        FormCard.FormCard {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0

                FormCard.FormHeader {
                    title: i18n("General")
                }

                FormCard.FormButtonDelegate {
                    id: aboutButton
                    text: i18n("About")
                    onClicked: applicationWindow().pageStack.push("qrc:/AboutPage.qml")
                }

                FormCard.FormDelegateSeparator { above: aboutButton; below: mmsButton }

                FormCard.FormButtonDelegate {
                    id: mmsButton
                    text: i18n("Multimedia Messages (MMS)")
                    onClicked: applicationWindow().pageStack.push("qrc:/settings/MMSSettingsPage.qml")
                }

                FormCard.FormDelegateSeparator { above: mmsButton }

                FormCard.FormTextDelegate {
                    id: restoreButton
                    text: i18n("Restore defaults")

                    trailing: Controls.Button {
                        text: i18n("Reset")
                        onClicked: {
                            const server = SettingsManager.mmsc
                            const proxy = SettingsManager.mmsProxy
                            chatListModel.restoreDefaults()
                            SettingsManager.mmsc = server
                            SettingsManager.mmsProxy = proxy
                        }
                    }
                }
            }
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            ColumnLayout {
                spacing: 0

                FormCard.FormHeader {
                    title: i18n("Appearance")
                }

                FormCard.FormSwitchDelegate {
                    id: customMessageColors
                    checked: SettingsManager.customMessageColors
                    text: i18n("Use custom colors for messages")
                    onToggled: SettingsManager.customMessageColors = checked
                }

                FormCard.FormDelegateSeparator { above: customMessageColors }

                FormCard.FormTextDelegate {
                    id: incomingMessageColor
                    text: i18n("Incoming message color")
                    visible: SettingsManager.customMessageColors
                    trailing: Controls.Button {
                        width: Kirigami.Units.gridUnit * 1.5
                        height: Kirigami.Units.gridUnit * 1.5
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
                }

                FormCard.FormDelegateSeparator { visible: incomingMessageColor.visible }

                FormCard.FormTextDelegate {
                    id: outgoingMessageColor
                    text: i18n("Outgoing message color")
                    visible: SettingsManager.customMessageColors
                    trailing: Controls.Button {
                        width: Kirigami.Units.gridUnit * 1.5
                        height: Kirigami.Units.gridUnit * 1.5
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
                }

                FormCard.FormDelegateSeparator { visible: outgoingMessageColor.visible }

                FormCard.FormTextDelegate {
                    id: messageFontSize
                    text: i18n("Message font size")
                    trailing: Controls.SpinBox {
                        value: SettingsManager.messageFontSize
                        from: -3
                        to: 3
                        stepSize: 1
                        onValueModified: SettingsManager.messageFontSize = value
                    }
                }
            }
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            ColumnLayout {
                spacing: 0

                FormCard.FormHeader {
                    title: i18n("Notifications")
                }

                FormCard.FormCheckDelegate {
                    id: showSenderInfo
                    checked: SettingsManager.showSenderInfo
                    text: i18n("Show sender name / number")
                    onToggled: SettingsManager.showSenderInfo = checked
                }

                FormCard.FormCheckDelegate {
                    id: showMessageContent
                    checked: SettingsManager.showMessageContent
                    text: i18n("Show a preview of the message content")
                    onToggled: SettingsManager.showMessageContent = checked
                }

                FormCard.FormCheckDelegate {
                    id: showAttachments
                    checked: SettingsManager.showAttachments
                    text: i18n("Show attachment previews")
                    onToggled: SettingsManager.showAttachments = checked
                }

                FormCard.FormCheckDelegate {
                    id: ignoreTapbacks
                    checked: SettingsManager.ignoreTapbacks
                    text: i18n("Ignore tapbacks")
                    onToggled: SettingsManager.ignoreTapbacks = checked
                }
            }
        }
    }

    Kirigami.Dialog {
        id: colorDialog
        topPadding: 0
        bottomPadding: 0
        leftPadding: 0
        rightPadding: 0

        property string field
        property color selected

        title: i18n("Choose a color")
        standardButtons: Kirigami.Dialog.Cancel | Kirigami.Dialog.Apply

        onApplied: {
            SettingsManager[colorDialog.field] = colorDialog.selected;
            colorDialog.close();
        }

        Grid {
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
    }
}
