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
    title: i18n("MMS Settings")

    leftPadding: 0
    rightPadding: 0
    topPadding: Kirigami.Units.gridUnit
    bottomPadding: Kirigami.Units.gridUnit

    width: applicationWindow().width
    Kirigami.ColumnView.fillWidth: true

    ColumnLayout {
        spacing: 0

        FormCard.FormCard {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0

                FormCard.FormHeader {
                    title: i18n("Multimedia messages (MMS)")
                }

                FormCard.FormTextFieldDelegate {
                    id: mmsc
                    placeholderText: "http://example.com/mms/wapenc"
                    text: SettingsManager.mmsc
                    label: i18n("MMSC")
                    onTextChanged: SettingsManager.mmsc = text.trim()
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormTextFieldDelegate {
                    id: mmsProxy
                    placeholderText: "proxy.example.com"
                    text: SettingsManager.mmsProxy
                    label: i18n("Proxy")
                    onTextChanged: SettingsManager.mmsProxy = text.trim()
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormTextDelegate {
                    id: mmsPort
                    text: i18n("Port")
                    trailing: Controls.SpinBox {
                        value: SettingsManager.mmsPort
                        from: 0
                        to: 65535
                        stepSize: 1
                        onValueModified: SettingsManager.mmsPort = value
                        textFromValue: function(value, locale) {
                            return value
                        }
                    }
                }

                FormCard.FormDelegateSeparator { below: requestDeliveryReports }

                FormCard.FormCheckDelegate {
                    id: requestDeliveryReports
                    checked: SettingsManager.requestDeliveryReports
                    text: i18n("Request delivery reports")
                    onToggled: SettingsManager.requestDeliveryReports = checked
                }

                FormCard.FormCheckDelegate {
                    id: requestReadReports
                    checked: SettingsManager.requestReadReports
                    text: i18n("Request read reports")
                    onToggled: SettingsManager.requestReadReports = checked
                }

                FormCard.FormCheckDelegate {
                    id: shareDeliveryStatus
                    checked: SettingsManager.shareDeliveryStatus
                    text: i18n("Share delivery status")
                    onToggled: SettingsManager.shareDeliveryStatus = checked
                }

                FormCard.FormCheckDelegate {
                    id: shareReadStatus
                    checked: SettingsManager.shareReadStatus
                    text: i18n("Share read status")
                    onToggled: SettingsManager.shareReadStatus = checked
                    enabled: false
                }

                FormCard.FormCheckDelegate {
                    id: autoDownload
                    checked: SettingsManager.autoDownload
                    text: i18n("Auto download messages")
                    onToggled: SettingsManager.autoDownload = checked
                }

                FormCard.FormCheckDelegate {
                    id: autoDownloadContactsOnly
                    visible: SettingsManager.autoDownload
                    checked: SettingsManager.autoDownloadContactsOnly
                    text: i18n("Auto download messages for existing contacts only")
                    onToggled: SettingsManager.autoDownloadContactsOnly = checked
                    enabled: SettingsManager.autoDownload == true
                }

                FormCard.FormTextDelegate {
                    id: totalMaxAttachmentSize
                    text: i18n("Max message size (KiB)")
                    trailing: Controls.SpinBox {
                        value: SettingsManager.totalMaxAttachmentSize
                        from: 100
                        to: 5000
                        stepSize: 50
                        onValueModified: SettingsManager.totalMaxAttachmentSize = value
                    }
                }

                FormCard.FormTextDelegate {
                    id: maxAttachments
                    text: i18n("Max attachments")
                    trailing: Controls.SpinBox {
                        value: SettingsManager.maxAttachments
                        from: 1
                        to: 25
                        onValueModified: SettingsManager.maxAttachments = value
                    }
                }

                FormCard.FormCheckDelegate {
                    id: autoCreateSmil
                    checked: SettingsManager.autoCreateSmil
                    text: i18n("Auto create SMIL")
                    onToggled: SettingsManager.autoCreateSmil = checked
                }

                FormCard.FormCheckDelegate {
                    id: groupConversation
                    checked: SettingsManager.groupConversation
                    text: i18n("Default to group conversations")
                    onToggled: SettingsManager.groupConversation = checked
                }
            }
        }
    }
}
