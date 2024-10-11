// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import spacebarfakeserver as Fakeserver

Kirigami.ApplicationWindow {
    id: root

    width: 700
    height: 600
    visibility: "Windowed"

    title: "Test Spacebar"

    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None
    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn

    pageStack.initialPage: Kirigami.ScrollablePage {
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        topPadding: Kirigami.Units.gridUnit
        leftPadding: 0
        rightPadding: 0
        bottomPadding: Kirigami.Units.gridUnit

        ColumnLayout {
            spacing: 0

            FormCard.FormHeader {
                title: "Send a message"
            }

            FormCard.FormCard {
                FormCard.FormTextFieldDelegate {
                    id: messageField
                    label: "Message Text"
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormTextFieldDelegate {
                    id: numberField
                    label: "Number"
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormButtonDelegate {
                    text: "Send"
                    icon.name: "document-send"
                    onClicked: {
                        Fakeserver.Server.sendMessage(messageField.text, numberField.text);
                    }
                }
            }

            FormCard.FormHeader {
                title: "Output log"
            }

            FormCard.FormCard {
                QQC2.TextArea {
                    text: Fakeserver.Server.messageLog
                    readOnly: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 15
                    Layout.fillWidth: true
                }
            }
        }
    }
}
