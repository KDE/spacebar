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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.1 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.telepathy 0.1 as KTp

Kirigami.Page {
    anchors.fill: parent

    title: "Your Conversations"

    mainAction: Kirigami.Action {
        text: "Start New Conversation"
        iconName: "document-edit"

        onTriggered: {
            if (root.pageStack.depth == 2) {
                root.pageStack.pop();
            }
            root.pageStack.push(newConversationPageComponent);
            print("Action button clicked")
        }
    }

    Loader {
        anchors.fill: parent
        active: telepathyManager.ready
        sourceComponent: mainModelComponent
    }

    Component {
        id: mainModelComponent

        ColumnLayout {
            id: rootLayout
            anchors.fill: parent

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true

                model: PlasmaCore.SortFilterModel {
                    id: plasmaSortFilterModel
                    sortRole: "lastMessageDate"
                    sortOrder: Qt.DescendingOrder
                    sourceModel: KTp.MainLogModel {
                        id: mainModel

                        onNewRequestedChannel: {
                            if (root.pageStack.currentPage.pageName === "newConversationPage" || openIncomingChannel) {
                                root.pageStack.pop();
                                root.pageStack.push(conversationPageComponent);
                                root.pageStack.currentPage.conversation = mainModel.data(index.row, "conversation");
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
                        if (root.pageStack.depth == 2) {
                            root.pageStack.pop();
                        }
                        root.pageStack.push(conversationPageComponent);
                        root.pageStack.currentItem.conversation = model.conversation;

                        // If the account is online, request a channel
                        if (mainModel.canChat(accountId)) {
                            mainModel.startChat(accountId, contactId);
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "white"
                        opacity: 0.8
                        visible: model.hasUnreadMessages
                    }

                    ColumnLayout {
                        id: messageLayout
                        width: parent.width

                        PlasmaExtras.Heading {
                            Layout.fillWidth: true

                            text: {
                                var t = model.contactDisplayName === "" ? model.contactId : model.contactDisplayName;
                                if (model.hasUnreadMessages) {
                                    t += " ";
                                    t += i18nc("N unread messages", "(%1 unread)", model.unreadMessagesCount);
                                }

                                return t;
                            }
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            level: 4
                        }
                        Kirigami.Label {
                            Layout.fillWidth: true

                            text: model.lastMessageText
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                            maximumLineCount: 2
                        }
                        Kirigami.Label {
                            Layout.fillWidth: true

                            text: Qt.formatDateTime(model.lastMessageDate)
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }
                    }
                }
            }
        }
    }
}
