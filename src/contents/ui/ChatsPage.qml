// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.12 as Kirigami
import QtQuick.Controls 2.4 as Controls

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: chatPage
    title: ChatListModel.ready ? i18n("Chats") : i18n("Loading...")
    supportsRefreshing: true
    actions {
        main: Kirigami.Action {
            text: i18n("New Conversation")
            onTriggered: pageStack.push("qrc:/NewConversationPage.qml")
            icon.name: "contact-new"
        }
    }

    onRefreshingChanged: {
        if (refreshing) {
            ChatListModel.fetchChats()
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: i18n("No chats yet")
        helpfulAction: actions.main

        visible: listView.count === 0
    }

    Connections {
        target: ChatListModel
        onChatStarted: (messageModel) => {
            Qt.callLater(pageStack.push, "qrc:/MessagesPage.qml", {"messageModel": messageModel})
        }
        onChatsFetched: {
            chatPage.refreshing = false
        }
    }

    ListView {
        id: listView
        model: ChatListModel

        delegate: Kirigami.AbstractListItem {
            id: delegateRoot

            required property string displayName
            required property string phoneNumber
            required property string lastContacted
            required property int unreadMessages
            required property var photo
            required property string lastMessage

            checkable: false
            highlighted: false

            height: Kirigami.Units.gridUnit * 3
            contentItem: RowLayout {
                RoundImage {
                    id: photo
                    height: parent.height * 0.8
                    width: height
                    smooth: true
                    source: delegateRoot.photo
                }

                ColumnLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignLeft
                    Kirigami.Heading {
                        level: 4
                        id: nameLabel
                        text: delegateRoot.displayName || delegateRoot.phoneNumber
                    }
                    Text {
                        id: lastMessage
                        text: delegateRoot.lastMessage
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
                    visible: delegateRoot.unreadMessages !== 0
                    height: Kirigami.Units.gridUnit * 1.2
                    width: number.width + 5 < height ? height: number.width + 5
                    radius: height * 0.5
                    color: Kirigami.Theme.highlightColor
                    Controls.Label {
                        id: number
                        anchors.centerIn: parent
                        visible: delegateRoot.unreadMessages !== 0
                        text: delegateRoot.unreadMessages
                        color: Qt.rgba(1, 1, 1, 1)
                    }
                }
            }

            onClicked: {
                // mark as read first, so data is correct when the model is initialized. This saves us a model reset
                ChatListModel.markChatAsRead(delegateRoot.phoneNumber);
                ChatListModel.startChat(delegateRoot.phoneNumber);
            }
        }
    }
}
