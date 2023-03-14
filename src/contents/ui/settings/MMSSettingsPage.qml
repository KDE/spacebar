// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

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

        MobileForm.FormCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Multimedia messages (MMS)")
                }

                MobileForm.FormTextFieldDelegate {
                    id: mmsc
                    placeholderText: "http://example.com/mms/wapenc"
                    text: SettingsManager.mmsc
                    label: i18n("MMSC")
                    onTextChanged: SettingsManager.mmsc = text.trim()
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextFieldDelegate {
                    id: mmsProxy
                    placeholderText: "proxy.example.com"
                    text: SettingsManager.mmsProxy
                    label: i18n("Proxy")
                    onTextChanged: SettingsManager.mmsProxy = text.trim()
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormTextDelegate {
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

                MobileForm.FormDelegateSeparator { below: requestDeliveryReports }

                MobileForm.FormCheckDelegate {
                    id: requestDeliveryReports
                    checked: SettingsManager.requestDeliveryReports
                    text: i18n("Request delivery reports")
                    onToggled: SettingsManager.requestDeliveryReports = checked
                }

                MobileForm.FormCheckDelegate {
                    id: requestReadReports
                    checked: SettingsManager.requestReadReports
                    text: i18n("Request read reports")
                    onToggled: SettingsManager.requestReadReports = checked
                }

                MobileForm.FormCheckDelegate {
                    id: shareDeliveryStatus
                    checked: SettingsManager.shareDeliveryStatus
                    text: i18n("Share delivery status")
                    onToggled: SettingsManager.shareDeliveryStatus = checked
                }

                MobileForm.FormCheckDelegate {
                    id: shareReadStatus
                    checked: SettingsManager.shareReadStatus
                    text: i18n("Share read status")
                    onToggled: SettingsManager.shareReadStatus = checked
                    enabled: false
                }

                MobileForm.FormCheckDelegate {
                    id: autoDownload
                    checked: SettingsManager.autoDownload
                    text: i18n("Auto download messages")
                    onToggled: SettingsManager.autoDownload = checked
                }

                MobileForm.FormCheckDelegate {
                    id: autoDownloadContactsOnly
                    visible: SettingsManager.autoDownload
                    checked: SettingsManager.autoDownloadContactsOnly
                    text: i18n("Auto download messages for existing contacts only")
                    onToggled: SettingsManager.autoDownloadContactsOnly = checked
                    enabled: SettingsManager.autoDownload == true
                }

                MobileForm.FormTextDelegate {
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

                MobileForm.FormTextDelegate {
                    id: maxAttachments
                    text: i18n("Max attachments")
                    trailing: Controls.SpinBox {
                        value: SettingsManager.maxAttachments
                        from: 1
                        to: 25
                        onValueModified: SettingsManager.maxAttachments = value
                    }
                }

                MobileForm.FormCheckDelegate {
                    id: autoCreateSmil
                    checked: SettingsManager.autoCreateSmil
                    text: i18n("Auto create SMIL")
                    onToggled: SettingsManager.autoCreateSmil = checked
                }

                MobileForm.FormCheckDelegate {
                    id: groupConversation
                    checked: SettingsManager.groupConversation
                    text: i18n("Default to group conversations")
                    onToggled: SettingsManager.groupConversation = checked
                }
            }
        }
    }
}
