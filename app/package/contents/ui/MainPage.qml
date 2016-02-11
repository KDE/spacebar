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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.mobilecomponents 0.2 as MobileComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.telepathy 0.1 as KTp

MobileComponents.Page {
    anchors.fill: parent

//     MobileComponents.RefreshableScrollView {
        ColumnLayout {
            id: rootLayout
            anchors.fill: parent

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                model: KTp.MainLogModel {
                    id: mainModel

                    Component.onCompleted: {
                        mainModel.setAccountManager(telepathyManager.accountManager);
                    }
                }

                delegate: PlasmaComponents.ListItem {
                    enabled: true

                    onClicked: {
                        root.pageStack.pop();
                        root.pageStack.push(conversationPageComponent);
                        mainModel.startChat(accountId, contactId);
                        root.pageStack.currentPage.conversation = model.conversation;
                    }

                    ColumnLayout {
                        Layout.fillWidth: true

                        PlasmaExtras.Heading {
                            wrapMode: Text.WordWrap
                            text: model.contactId
                            level: 4
                        }
                        PlasmaComponents.Label {
                            wrapMode: Text.WordWrap
                            text: model.lastMessageText
                        }
                        PlasmaComponents.Label {
                            wrapMode: Text.WordWrap
                            text: Qt.formatDateTime(model.lastMessageDate)
                        }

                    }
                }
            }
        }
//     }
}
