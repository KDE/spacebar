// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: msgPage

    title: messageModel && (messageModel.person.name || messageModel.person.phoneNumber || messageModel.phoneNumber)
    property MessageModel messageModel;

    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: premiumWarning
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Warning
            text: i18n("Texting this premium SMS number might cause you to be charged money")
            visible: messageModel && Utils.isPremiumNumber(messageModel.phoneNumber)
        }
    }

    ListView {
        id: listView
        model: messageModel
        spacing: Kirigami.Units.largeSpacing

        // when there is a new message or the the chat is first viewed, go to the bottom
        onCountChanged: contentY = contentHeight

        add: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }

        section.property: "date"
        section.delegate: Controls.Control {
            bottomPadding: Kirigami.Units.largeSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            contentItem: Kirigami.ListSectionHeader {
                Text {
                    text: Qt.formatDate(section, Qt.locale().dateFormat(Locale.LongFormat))
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        delegate: Item {
            id: delegateParent
            width: listView.width
            height: rect.height

            Component.onCompleted: {
                // Avoid unnecessary invocations

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

                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                
                anchors.margins: Kirigami.Units.largeSpacing
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined
                
                radius: Kirigami.Units.smallSpacing
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: Kirigami.Units.devicePixelRatio
                
                color: model.sentByMe ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor
                
                height: content.height + Kirigami.Units.largeSpacing * 2
                width: content.width + Kirigami.Units.largeSpacing * 2

                ColumnLayout {
                    spacing: 0
                    id: content
                    anchors.centerIn: parent
                    
                    // adjust for highlight color
                    property color textColor: model.sentByMe ? Qt.rgba(255, 255, 255, 0.9) : Kirigami.Theme.textColor
                    
                    // message contents
                    Controls.Label {
                        Layout.alignment: Qt.AlignTop
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 5
                        Layout.maximumWidth: delegateParent.width * 0.7
                        text: model.text ? model.text : " " // guarantee there is text so that height is maintained
                        wrapMode: Text.Wrap
                        textFormat: Text.StyledText
                        linkColor: model.sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.linkColor
                        color: content.textColor
                    }

                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        Item { Layout.fillWidth: true }
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignRight
                            implicitHeight: Math.round(Kirigami.Units.gridUnit * 0.7)
                            implicitWidth: implicitHeight
                            source: {
                                if (visible) {
                                    switch (model.deliveryState) {
                                    case MessageModel.Unknown:
                                        return undefined;
                                    case MessageModel.Pending:
                                        return "content-loading-symbolic";
                                    case MessageModel.Sent:
                                        return "answer-correct";
                                    case MessageModel.Failed:
                                        return "error"
                                    }
                                }

                                return undefined
                            }

                            visible: model.sentByMe
                            color: content.textColor
                        }
                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            text: Qt.formatTime(model.time, Qt.DefaultLocaleShortDate)
                            font: Kirigami.Theme.smallFont
                            color: content.textColor
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: rect
                onPressAndHold: {
                    menu.message = model.text
                    menu.open()
                }
            }
        }
    }

    Kirigami.OverlayDrawer {
        id: menu

        property string message

        edge: Qt.BottomEdge

        contentItem: ColumnLayout {
            Kirigami.BasicListItem {
                text: i18n("Copy message")
                icon: "edit-copy"
                onClicked: {
                    Utils.copyTextToClipboard(menu.message)
                    menu.close()
                }
            }
        }
    }

    footer: Kirigami.ActionTextField {
        id: field
        height: Kirigami.Units.gridUnit * 2
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
