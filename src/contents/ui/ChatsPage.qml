// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.spacebar

Kirigami.ScrollablePage {
    id: chatPage
    title: i18n("Chats")

    property var conversations: []
    property bool editing: false

    function setConversations (phoneNumberList) {
        if (conversations.length === 0) {
            editing = true
        }
        const index = conversations.indexOf(phoneNumberList)
        if (index === -1) {
            conversations.push(phoneNumberList)
        } else {
            conversations.splice(index, 1)
        }
        conversations = conversations
        if (conversations.length === 0) {
            editing = false
        }
    }

    onWidthChanged: ChatListModel.setCharacterLimit(applicationWindow().width)
    
    actions: [
        Kirigami.Action {
            visible: !Kirigami.Settings.isMobile
            icon.name: "contact-new"
            text: i18n("New Conversation")
            onTriggered: pageStack.push("qrc:/NewConversationPage.qml")
        },
        Kirigami.Action {
            displayHint: Kirigami.DisplayHint.IconOnly
            icon.name: "settings-configure"
            text: i18nc("Configuring application settings", "Settings")
            onTriggered: {
                applicationWindow().pageStack.push("qrc:/settings/SettingsPage.qml", {"chatListModel": ChatListModel})
            }
        },
        Kirigami.Action {
            displayHint: Kirigami.DisplayHint.IconOnly
            icon.name: "delete"
            text: i18nc("Deleting a conversation", "Delete")
            onTriggered: promptDialog.open()
            visible: editing === true
        }
    ]

    ListView {
        id: listView
        model: ChatListModel
        reuseItems: false

        Connections {
            target: ChatListModel
            function onChatStarted (messageModel) {
                // Don't open two MessagesPages at the same time
                if (pageStack.currentItem.hasOwnProperty("messageModel")) {
                    pageStack.pop()
                }

                Qt.callLater(pageStack.push, "qrc:/MessagesPage.qml", {"messageModel": messageModel})
            }

            function onChatsFetched() {
                loading.visible = false
            }
        }
        
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: i18nc("Selecting recipients from contacts list", "Create a chat")
            icon.name: "dialog-messages"
            helpfulAction: actions[0]
            visible: !loading.visible && listView.count === 0
        }

        Controls.BusyIndicator {
            id: loading
            anchors.centerIn: parent
            visible: listView.count === 0
            running: visible
            width: Kirigami.Units.iconSizes.huge
            height: width
        }
        
        // mobile add action
        FloatingActionButton {
            anchors.fill: parent
            iconName: "list-add"
            onClicked: pageStack.push("qrc:/NewConversationPage.qml")
            visible: Kirigami.Settings.isMobile
        }

        delegate: Controls.ItemDelegate {
            id: delegateRoot

            required property string displayName
            required property var phoneNumberList
            required property int unreadMessages
            required property string lastMessage
            required property bool lastSentByMe
            required property var lastAttachment
            required property string lastContacted
            required property bool isContact

            property var attachments: lastAttachment ? JSON.parse(lastAttachment) : []
            property var image: attachments.find(o => o.mimeType.indexOf("image/") >= 0)
            property bool selected: conversations.indexOf(delegateRoot.phoneNumberList) >= 0

            width: listView.width

            contentItem: Loader {
                sourceComponent: Component {
                    RowLayout {
                        spacing: Kirigami.Units.largeSpacing

                        Components.Avatar {
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            Layout.topMargin: Kirigami.Units.largeSpacing
                            Layout.bottomMargin: Kirigami.Units.largeSpacing
                            source: isContact ? "image://avatar/" + Utils.phoneNumberListToString(delegateRoot.phoneNumberList) : ""
                            name: delegateRoot.displayName
                            imageMode: Components.Avatar.ImageMode.AdaptiveImageOrInitals
                            initialsMode: isContact ? Components.Avatar.InitialsMode.UseInitials : Components.Avatar.InitialsMode.UseIcon

                            Rectangle {
                                anchors.fill: parent
                                radius: width * 0.5
                                color: Kirigami.Theme.highlightColor
                                visible: selected

                                Kirigami.Icon {
                                    anchors.fill: parent
                                    source: "checkbox"
                                    color: Kirigami.Theme.highlightedTextColor
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true

                            spacing: 0
                            Kirigami.Heading {
                                id: nameLabel
                                level: 5
                                type: Kirigami.Heading.Type.Normal
                                Layout.fillWidth: true
                                text: delegateRoot.displayName
                                wrapMode: Text.WrapAnywhere
                                maximumLineCount: 1
                            }
                            Text {
                                id: lastMessage
                                Layout.fillWidth: true
                                text: (delegateRoot.lastSentByMe ? i18nc("Indicating that message was sent by you", "You") + ": " : "") + (delegateRoot.lastMessage || (delegateRoot.image ? i18nc("Indicating that message contains an image", "Picture") : ""))
                                wrapMode: Text.WrapAnywhere
                                textFormat: Text.PlainText
                                maximumLineCount: 1
                                elide: Qt.ElideRight
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize - 2
                                font.family: "Noto Sans, Noto Color Emoji"
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
                            Layout.preferredWidth: delegateRoot.image ? Kirigami.Units.iconSizes.smallMedium * 2 : 0
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium * 2
                            asynchronous: true
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
                            Layout.minimumWidth: Kirigami.Units.smallSpacing * 13
                            horizontalAlignment: Text.AlignRight
                            topPadding: Kirigami.Units.largeSpacing * 2
                            text: delegateRoot.lastContacted
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize - 2
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
                onLoaded: ChatListModel.fetchChatDetails(delegateRoot.phoneNumberList)
            }

            onPressAndHold: setConversations(delegateRoot.phoneNumberList)

            onClicked: {
                if (editing) {
                    setConversations(delegateRoot.phoneNumberList)
                } else {
                    // mark as read first, so data is correct when the model is initialized. This saves us a model reset
                    if (delegateRoot.unreadMessages > 0) {
                        ChatListModel.markChatAsRead(delegateRoot.phoneNumberList)
                        delegateRoot.unreadMessages = 0
                    }
                    ChatListModel.startChat(delegateRoot.phoneNumberList)
                }
            }
        }
    }

    Kirigami.PromptDialog {
        id: promptDialog
        title: i18np("Delete this conversation?", "Delete %1 conversations?", conversations.length)
        subtitle: i18n("This is permanent and can't be undone")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: {
            conversations.forEach(conversation => {
                ChatListModel.deleteChat(conversation)
            })
            conversations = []
            editing = false
        }
        onRejected: close()
    }

    footer: null
}
