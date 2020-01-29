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

import QtQuick 2.3
import QtQuick.Controls 2.3 as Controls
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.1 as Kirigami
import org.kde.telepathy 0.1 as KTp

Kirigami.ScrollablePage {
    title: "Your Conversations"

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    mainAction: Kirigami.Action {
        text: "Start New Conversation"
        iconName: "document-edit"

        onTriggered: {
            if (root.pageStack.depth === 2) {
                root.pageStack.pop();
            }
            root.pageStack.push(newConversationPageComponent);
            print("Action button clicked")
        }
    }

    Connections {
        target: root

        onStartChat: {
            mainModel.startChat(personUri)
        }
    }

    ListView {
        anchors.fill: parent
        clip: true

        Controls.Label {
            text: i18n("No conversations yet")
            anchors.centerIn: parent
            visible: plasmaSortFilterModel.count === 0 && telepathyManager.ready
        }

        Controls.BusyIndicator {
            running: !telepathyManager.ready
            anchors.centerIn: parent
            width: Kirigami.Units.gridUnit * 5
        }

        model: PlasmaCore.SortFilterModel {
            id: plasmaSortFilterModel
            sortRole: "lastMessageDate"
            sortOrder: Qt.DescendingOrder
            sourceModel: KTp.MainLogModel {
                id: mainModel

                onNewRequestedChannel: {
                    if (root.pageStack.currentItem.pageName === "newConversationPage" || openIncomingChannel) {
                        root.pageStack.replace(conversationPageComponent);
                        root.pageStack.currentItem.conversation = mainModel.data(index.row, "conversation");
                        openIncomingChannel = false;
                    }
                }

                Component.onCompleted: {
                    telepathyManager.registerClient(mainModel, "SpaceBar");
                    telepathyManager.registerClient(mainModel.observerProxy(), "SpaceBarObserverProxy");
                    mainModel.setAccountManager(telepathyManager.accountManager);
                }

                Component.onDestruction: {
                    telepathyManager.unregisterClient(mainModel);
                }
            }
        }

        delegate: Kirigami.AbstractListItem {
            supportsMouseEvents: true

            onClicked: {
                if (root.pageStack.depth === 2) {
                    root.pageStack.pop();
                }
                root.pageStack.push(conversationPageComponent);
                root.pageStack.currentItem.conversation = model.conversation;

                // If the account is online, request a channel
                if (mainModel.canChat(accountId)) {
                    mainModel.startChat(accountId, contactId);
                }
            }

            RowLayout {
                Kirigami.Icon {
                    source: model.chatPicture ? model.chatPicture : "user-identity"
                    width: Kirigami.Units.gridUnit * 4
                    height: width
                }

                ColumnLayout {
                    id: messageLayout
                    width: parent.width

                    Kirigami.Heading {
                        Layout.fillWidth: true

                        text: model.contactDisplayName ? model.contactDisplayName : model.contactId;
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 1
                        level: 4
                    }
                    Controls.Label {
                        Layout.fillWidth: true

                        text: model.lastMessageText
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 2
                    }
                    Controls.Label {
                        Layout.fillWidth: true

                        text: Qt.formatDateTime(model.lastMessageDate)
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                        maximumLineCount: 1
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
                Rectangle {
                    visible: model.hasUnreadMessages

                    Layout.preferredHeight: Kirigami.Units.gridUnit * 1.25
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 1.25

                    id: counterCircle
                    radius: counterCircle.height * 0.5
                    color: Kirigami.Theme.positiveTextColor

                    Text {
                        id: msgCounter
                        text: model.unreadMessagesCount
                        color: "white"
                        anchors.centerIn: parent
                    }
                }
            }
        }
    }
}
