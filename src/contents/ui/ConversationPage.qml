/**
 *   Copyright 2016 Martin Klapetek <mklapetek@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Controls 2.4 as Controls
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.4 as Kirigami
import org.kde.telepathy 0.1


Kirigami.ScrollablePage {
    id: conversationPage

    title: conversation.title ? conversation.title : i18n("Chat")

    // This is somewhat a hack, the type should be Conversation
    // but QML does not allow for uncreatable types to be property
    // types, so needs to be QtObject instead
    property QtObject conversation
    property string pageName: "conversationPage"

    signal insertEmoji(var emoji);

    signal focusTextInput();

    // TODO
    // Find some place to display conversation.presenceIcon

    ListView {
        id: view
        property bool followConversation: true

        EmojisModel {
            id: emojisModel
        }

        Layout.fillWidth: true
        Layout.fillHeight: true

        section.property: "senderAlias"
        section.delegate: Controls.Label {
            anchors.right: parent.right
            anchors.left: parent.left
            height: paintedHeight * 1.5
            horizontalAlignment: section === conversation.title ? Text.AlignLeft : Text.AlignRight
            verticalAlignment: Text.AlignBottom
            text: section
            font.bold: true
        }
        clip: true

        add: Transition {
            id: addTrans
            NumberAnimation {
                properties: "x"
                //FIXME: this doesn't seem to do what it should
                from: addTrans.ViewTransition.item.isIncoming ? -100 : 100
                duration: 60

            }
            PropertyAnimation {
                properties: "opacity"
                from: 0.0
                to: 1.0
                duration: 60
            }
        }

        //we need this so that scrolling down to the last element works properly
        //this means that all the list is in memory
        cacheBuffer: Math.max(0, contentHeight)

        delegate: Loader {
            Component.onCompleted: {
                switch (model.type) {
                    case MessagesModel.MessageTypeOutgoing:
                    case MessagesModel.MessageTypeIncoming:
                        source = "TextDelegate.qml"
                        break;
                    case MessagesModel.MessageTypeAction:
                        source = "ActionDelegate.qml";
                        break;
                }
            }
        }

        footer: Controls.Label {
            id: statusMessageLabel
            Layout.fillWidth: true
            Layout.maximumHeight: height
            text: conversation.isContactTyping ? i18nc("Contact is composing a message",
                                                       "%1 is typing...", conversation.title) : ""
            height: text.length == 0 ? 0 : paintedHeight

            Behavior on height {
                NumberAnimation {}
            }
        }


        model: conversation.messages

        Connections {
            target: conversation.messages

            onRowsInserted: {
                if (view.followConversation) {
                    view.positionViewAtEnd();
                }
            }
        }

        onMovementEnded: followConversation = atYEnd //we only follow the conversation if moved to the end

        onContentHeightChanged: {
            if (followConversation && contentHeight > height) {
                view.positionViewAtEnd()
            }
        }

        onAtYBeginningChanged: {
            if (atYBeginning) {
                model.fetchMoreHistory();
            }
        }

        Component.onCompleted: {
            conversation.messages.visibleToUser = true;
        }

        Component.onDestruction: {
            conversation.messages.visibleToUser = false;
        }
    }

    footer: ColumnLayout {
        Controls.Pane {
            Layout.fillWidth: true
            layer.enabled: true
            layer.effect: DropShadow {
                color: Kirigami.Theme.disabledTextColor
                samples: 20
                spread: 0.3
                cached: true // element is static
            }
            padding: 0
            wheelEnabled: true
            background: Rectangle {
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: Kirigami.Theme.backgroundColor
            }

            RowLayout {
                anchors.fill: parent
                EmojiTextArea {
                    id: emojiTextArea
                    Layout.fillWidth: true
                    Layout.minimumHeight: sendButton.height
                    Layout.maximumHeight: emojiTextArea.lineCount * emojiTextArea.lineSpacing + units.largeSpacing

                    Connections {
                        target: conversationPage
                        onInsertEmoji: {
                            emojiTextArea.insert(emojiTextArea.cursorPosition, emoji + " ");
                        }
                        onFocusTextInput: {
                            emojiTextArea.forceActiveFocus();
                        }
                    }
                }

                Controls.ToolButton {
                    id: emojisButton

                    icon.name: "face-smile"
                    checkable: true
                }

                Controls.ToolButton {
                    id: sendButton
                    enabled: conversation !== null
                    icon.name: "document-send"

                    onClicked: {
                        view.model.sendNewMessage(emojiTextArea.getEmojiText())
                        emojiTextArea.text = "";
                    }
                }
            }
        }
        GridView {
            id: emojisGridView
            Layout.fillWidth: true
            height: conversationPage.height / 3
            Layout.leftMargin: conversationPage.width % cellWidth * 0.5
            Layout.rightMargin: Layout.leftMargin
            clip: true
            visible: emojisButton.checked

            property int iconSize: Kirigami.Units.iconSizes.medium

            model: emojisModel
            cellWidth: Kirigami.Units.gridUnit * 3
            cellHeight: cellWidth

            delegate: MouseArea {
                height: emojisGridView.iconSize
                width: emojisGridView.iconSize

                onClicked: {
                    conversationPage.insertEmoji(model.emojiText);
                    emojisButton.checked = false
                    conversationPage.focusTextInput();
                }

                Kirigami.Icon {
                    height: emojisGridView.iconSize
                    width: emojisGridView.iconSize
                    source: model.emojiFullPath
                }
            }
        }
    }
}
