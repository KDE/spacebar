// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.4 as Controls

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: chatPage
    title: i18n("Chats")
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
        function onChatStarted (messageModel) {
            // Don't open two MessagesPages at the same time
            if (pageStack.currentItem.hasOwnProperty("messageModel")) {
                pageStack.pop()
            }

            Qt.callLater(pageStack.push, "qrc:/MessagesPage.qml", {"messageModel": messageModel})
        }
        function onChatsFetched () {
            chatPage.refreshing = false
        }
    }

    ListView {
        id: listView
        model: ChatListModel

        reuseItems: true

        delegate: Kirigami.SwipeListItem {
            id: delegateRoot

            required property string displayName
            required property string phoneNumber
            required property string displayPhoneNumber
            required property string lastContacted
            required property int unreadMessages
            required property var photo
            required property string lastMessage

            checkable: false
            highlighted: false

            contentItem: RowLayout {
                Kirigami.Avatar {
                    id: photo
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    Layout.rightMargin: Kirigami.Units.largeSpacing

                    source: "image://avatar/" + delegateRoot.phoneNumber
                    name: delegateRoot.displayName || delegateRoot.displayPhoneNumber
                    imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
                    initialsMode: delegateRoot.displayName ? Kirigami.Avatar.UseInitials : Kirigami.Avatar.UseIcon
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    spacing: 0
                    Kirigami.Heading {
                        level: 3
                        Layout.fillWidth: true
                        id: nameLabel
                        text: delegateRoot.displayName || delegateRoot.displayPhoneNumber
                    }
                    Text {
                        id: lastMessage
                        Layout.fillWidth: true
                        text: delegateRoot.lastMessage
                        textFormat: Text.StyledText
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

            actions: [
                Kirigami.Action {
                    text: i18n("Delete chat")
                    icon.name: "delete"
                    onTriggered: {
                        ChatListModel.deleteChat(delegateRoot.phoneNumber)
                    }
                }
            ]

            onClicked: {
                // mark as read first, so data is correct when the model is initialized. This saves us a model reset
                ChatListModel.markChatAsRead(delegateRoot.phoneNumber);
                ChatListModel.startChat(delegateRoot.phoneNumber);
            }
        }
    }
}
