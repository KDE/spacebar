// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.2 as Kirigami
import QtQuick.Controls 2.4 as Controls

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    title: ChatListModel.ready ? i18n("Chats") : i18n("Loading...")

    actions {
        main: Kirigami.Action {
            text: i18n("New Conversation")
            onTriggered: pageStack.push("qrc:/NewConversationPage.qml")
            icon.name: "contact-new"
        }
    }

    Controls.Label {
        anchors.centerIn: parent
        text: i18n("No chats yet")
        visible: ChatListModel.count === 0
    }

    Connections {
        target: ChatListModel
        onChatStarted: {
            if (pageStack.depth > 1) {
                pageStack.pop()
            }
            pageStack.push("qrc:/MessagesPage.qml", {"messageModel": messageModel})
        }
    }

    ListView {
        model: ChatListModel

        delegate: Kirigami.AbstractListItem {
            checkable: false
            height: Kirigami.Units.gridUnit * 3
            contentItem: RowLayout {
                RoundImage {
                    id: photo
                    height: parent.height * 0.8
                    width: height
                    smooth: true
                    source: model.photo
                }

                ColumnLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignLeft
                    Kirigami.Heading {
                        level: 4
                        id: nameLabel
                        text: model.displayName || model.phoneNumber
                    }
                    Text {
                        id: lastMessage
                        text: model.lastMessage
                        maximumLineCount: 1
                        elide: Qt.ElideRight
                        color: Qt.tint(Kirigami.Theme.disabledTextColor, Kirigami.Theme.textColor)
                    }
                }

                // spacer
                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.alignment: Qt.AlignRight
                    visible: model.unreadMessages !== 0
                    height: Kirigami.Units.gridUnit * 1.2
                    width: number.width + 5 < height ? height: number.width + 5
                    radius: height * 0.5
                    color: Kirigami.Theme.highlightColor
                    Controls.Label {
                        id: number
                        anchors.centerIn: parent
                        visible: model.unreadMessages !== 0
                        text: model.unreadMessages
                        color: Qt.rgba(1, 1, 1, 1)
                    }
                }
            }

            onClicked: {
                ChatListModel.startChat(model.phoneNumber);
                ChatListModel.markChatAsRead(model.phoneNumber);
            }
        }
    }
}
