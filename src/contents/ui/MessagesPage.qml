// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as Controls

import org.kde.kirigami 2.12 as Kirigami

import org.kde.spacebar 1.0

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

                // This code is only for marking messages as read that arrived after opening the chat.
                // However we currently don't know the id of those messages.
                // For now we use the function to mark all messages as read, but this should actually use
                // messageModel.markMessageRead(model.id)

                if (visible && !model.sentByMe && !model.read) {
                    Qt.callLater(ChatListModel.markChatAsRead, messageModel.phoneNumber);
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
                    const isDarkTheme = Kirigami.ColorUtils.brightnessForColor(Kirigami.Theme.backgroundColor) === Kirigami.ColorUtils.Dark
                    const myColor = isDarkTheme ? "#395066" : "#3daee9"
                    const chatParterColor = isDarkTheme ? "#2c3e50" : "#e6e8e9"
                    model.sentByMe ? myColor : chatParterColor
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
                            text: Qt.formatTime(model.time, Qt.DefaultLocaleShortDate)
                        }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignRight
                            height: 15
                            implicitHeight: height
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
