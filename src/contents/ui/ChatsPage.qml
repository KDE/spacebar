// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import QtGraphicalEffects 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: chatPage
    title: i18n("Chats")
    supportsRefreshing: true
    
    actions {
        main: Kirigami.Action {
            visible: !Kirigami.Settings.isMobile
            text: i18n("New Conversation")
            onTriggered: pageStack.push("qrc:/NewConversationPage.qml", { isNew: true })
            icon.name: "contact-new"
        }
        
        contextualActions: [
            Kirigami.Action {
                displayHint: Kirigami.Action.IconOnly
                iconName: "settings-configure"
                text: i18n("Settings")
                onTriggered: {
                    applicationWindow().pageStack.push("qrc:/SettingsPage.qml", {"chatListModel": ChatListModel})
                }
            }
        ]
    }

    onRefreshingChanged: {
        if (refreshing) {
            ChatListModel.fetchChats()
        }
    }

    ListView {
        id: listView
        model: ChatListModel

        reuseItems: true
        
        Connections {
            target: ChatListModel
            function onChatStarted (messageModel) {
                let isNew = false

                // Don't open two MessagesPages at the same time
                if (pageStack.currentItem.hasOwnProperty("messageModel")) {
                    pageStack.pop()
                    isNew = true
                }

                Qt.callLater(pageStack.push, "qrc:/MessagesPage.qml", {"messageModel": messageModel, isNew: isNew})
            }
            function onChatsFetched () {
                chatPage.refreshing = false
            }
        }
        
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: i18n("Create a chat")
            icon.name: "dialog-messages"
            helpfulAction: actions.main

            visible: listView.count === 0
        }
        
        // mobile add action
        FloatingActionButton {
            anchors.fill: parent
            iconName: "list-add"
            onClicked: pageStack.push("qrc:/NewConversationPage.qml", { isNew: true })
            visible: Kirigami.Settings.isMobile
        }

        delegate: Kirigami.SwipeListItem {
            id: delegateRoot

            required property string displayName
            required property var phoneNumberList
            required property string lastContacted
            required property int unreadMessages
            required property string lastMessage
            required property bool lastSentByMe
            required property var lastAttachment
            required property bool isContact

            property var attachments: lastAttachment ? JSON.parse(lastAttachment) : []
            property var image: attachments.find(o => o.mimeType.indexOf("image/") >= 0)

            checkable: false
            highlighted: false
            separatorVisible: false
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            contentItem: RowLayout {
                Kirigami.Avatar {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                    source: isContact ? "image://avatar/" + Utils.phoneNumberListToString(delegateRoot.phoneNumberList) : ""
                    name: delegateRoot.displayName
                    imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
                    initialsMode: isContact ? Kirigami.Avatar.UseInitials : Kirigami.Avatar.UseIcon
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    spacing: 0
                    Kirigami.Heading {
                        id: nameLabel
                        level: 4
                        type: Kirigami.Heading.Type.Normal
                        Layout.fillWidth: true
                        text: delegateRoot.displayName
                        wrapMode: Text.WrapAnywhere
                        maximumLineCount: 1
                    }
                    Text {
                        id: lastMessage
                        Layout.fillWidth: true
                        text: (delegateRoot.lastSentByMe ? i18n("You: ") : "") + (delegateRoot.lastMessage || (delegateRoot.image ? i18n("Picture") : ""))
                        wrapMode: Text.WrapAnywhere
                        textFormat: Text.PlainText
                        maximumLineCount: 1
                        elide: Qt.ElideRight
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize - 2
                        color: Kirigami.Theme.disabledTextColor
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

                Image {
                    id: image
                    source: delegateRoot.image ? "file://" + ChatListModel.attachmentsFolder(delegateRoot.phoneNumberList) + "/" + delegateRoot.image.fileName : ""
                    fillMode: Image.PreserveAspectCrop
                    sourceSize.height: Kirigami.Units.iconSizes.smallMedium * 4
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium * 2
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium * 2
                    cache: false

                    // rounded corners on image
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: image.width
                            height: image.height
                            Rectangle {
                                anchors.fill: parent
                                radius: Kirigami.Units.smallSpacing
                            }
                        }
                    }
                }

                Text {
                    visible: !delegateRoot.image
                    topPadding: Kirigami.Units.largeSpacing * 2
                    text: delegateRoot.lastContacted
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize - 2
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Delete chat")
                    icon.name: "delete"
                    onTriggered: {
                        ChatListModel.deleteChat(delegateRoot.phoneNumberList)
                    }
                }
            ]

            onClicked: {
                // mark as read first, so data is correct when the model is initialized. This saves us a model reset
                if (delegateRoot.unreadMessages > 0) {
                    ChatListModel.markChatAsRead(delegateRoot.phoneNumberList)
                }
                ChatListModel.startChat(delegateRoot.phoneNumberList)
            }
        }
    }
}
