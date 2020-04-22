// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as Controls

import org.kde.kirigami 2.12 as Kirigami

import org.kde.spacebear 1.0

Kirigami.ScrollablePage {
    id: msgPage

    title: messageModel.person.name || messageModel.person.phoneNumber || messageModel.phoneNumber
    property MessageModel messageModel;

    Controls.BusyIndicator {
        anchors.centerIn: parent
        running: !messageModel.isReady
        visible: !messageModel.isReady
    }

    ListView {
        id: listView
        model: messageModel
        spacing: 10

        verticalLayoutDirection: ListView.BottomToTop

        add: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }

        delegate: Item {
            width: listView.width
            height: rect.height

            Component.onCompleted: {
                // Avoid unneccessary invocations
                // TODO: is this stupid?
                if (!model.sentByMe && !model.read) {
                    messageModel.markMessageRead(model.id)
                }
            }

            Kirigami.ShadowedRectangle {
                id: rect
                anchors.margins: 20
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined
                radius: 10
                shadow.size: 3
                color: {
                    if (model.sentByMe) {
                        var col = Kirigami.Theme.highlightColor
                        col.a = 0.1
                        Qt.tint(Kirigami.Theme.visitedLinkBackgroundColor, col)
                    } else {
                        Qt.lighter(Kirigami.Theme.visitedLinkBackgroundColor, 1.04)
                    }
                }
                height: content.height + 10
                width: content.width + 10
                ColumnLayout {
                    spacing: 0
                    id: content
                    anchors.centerIn: parent
                    Controls.Label {
                        Layout.alignment: Qt.AlignTop
                        text: model.text
                        wrapMode: Text.WordWrap
                        Layout.minimumWidth: 100
                        Layout.minimumHeight: 30
                        Layout.maximumWidth: msgPage.width * 0.7
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            color: Kirigami.Theme.disabledTextColor
                            text: Qt.formatTime(model.time, Qt.DefaultLocaleShortDate)
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignRight
                            height: 15
                            width: height
                            source: visible ? "answer-correct" : undefined
                            visible: model.delivered && model.sentByMe
                        }
                    }
                }
            }
        }
    }

    footer: Kirigami.ActionTextField {
        id: field
        height: 40
        placeholderText: i18n("Write Message...")
        onAccepted: text !== "" && sendAction.triggered()
        rightActions: [
            Kirigami.Action {
                id: sendAction
                text: i18n("Send")
                icon.name: "document-send"
                enabled: field.text !== ""
                onTriggered: {
                    messageModel.sendMessage(field.text)
                    field.text = ""
                }
            }
        ]
    }
}
