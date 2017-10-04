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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kirigami 2.1 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
// import org.kde.plasma.private.spacebar 1.0
import org.kde.telepathy 0.1

Kirigami.Page {
    id: conversationPage
    anchors.fill: parent

    // This is somewhat a hack, the type should be Conversation
    // but QML does not allow for uncreatable types to be property
    // types, so needs to be QtObject instead
    property QtObject conversation
    property string pageName: "conversationPage"

    signal insertEmoji(var emoji);

    signal focusTextInput();

    EmojisModel {
        id: emojisModel
    }


    Kirigami.OverlaySheet {
        id: emojisRect

        GridView {
            id: emojisGridView
            width: conversationPage.width
            height: conversationPage.height / 3
            clip: true

            property int iconSize: units.roundToIconSize(cellWidth)

            model: emojisModel
            cellWidth: Math.floor(width / 10)
            cellHeight: cellWidth

            delegate: MouseArea {
                height: emojisGridView.iconSize
                width: emojisGridView.iconSize

                onClicked: {
                    conversationPage.insertEmoji(model.emojiText);
                    emojisRect.close();
                    conversationPage.focusTextInput();
                }

                PlasmaCore.IconItem {
                    height: emojisGridView.iconSize
                    width: emojisGridView.iconSize
                    source: model.emojiFullPath
                }
            }
        }
    }

    Loader {
        anchors.fill: parent
        active: conversation !== null
        sourceComponent: conversationComponent
    }

    Component {
        id: conversationComponent

        ColumnLayout {
            anchors.fill: parent
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                PlasmaCore.IconItem {
                    source: conversation.presenceIcon
                }

                Kirigami.Label {
                    Layout.fillWidth: true
                    text: conversation.title

                }

                Button {
                    text: i18nc("Close an active conversation", "Close")

                    onClicked: {
                        conversation.messages.visibleToUser = false;
                        conversation.requestClose();
                        root.pageStack.pop();
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true

                height: 1
                color: "#888888" //FIXME use theme color
                opacity: 0.4
            }

            ListView {
                id: view
                property bool followConversation: true

                Layout.fillWidth: true
                Layout.fillHeight: true

                boundsBehavior: Flickable.StopAtBounds

                section.property: "senderAlias"
                section.delegate: Kirigami.Label {
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

            Kirigami.Label {
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

            RowLayout {

                EmojiTextArea {
                    id: emojiTextArea
                    Layout.fillWidth: true
                    Layout.minimumHeight: sendButton.height
                    Layout.maximumHeight: emojiTextArea.lineCount * emojiTextArea.lineSpacing + units.largeSpacing

                    emojisAutocompletionModel: emojisModel

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

                Button {
                    id: emojisButton

                    Layout.maximumWidth: implicitWidth / 2

                    text: ":)"

                    onClicked: {
                        emojisRect.open();
                    }
                }

                Button {
                    id: sendButton
                    enabled: conversation !== null
                    text: conversation.account !== null && conversation.account.online ?
                                    i18nc("Button label; Send message", "Send")
                                    : i18nc("Button label; Connect first and then send message", "Connect and Send")

                    onClicked: {
                        view.model.sendNewMessage(emojisHandler.getText())
                        messageTextField.text = "";
                    }
                }
            }
        }
    }
}
