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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as Controls
import org.kde.kirigami 2.1 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    width: 500
    height: 800
    visible: true

    signal startChat(string personUri)

    globalDrawer: Kirigami.GlobalDrawer {
        title: "SpaceBar"
        titleIcon: "org.kde.spacebar"
//         bannerImageSource: "banner.jpg"

        actions: [
            Kirigami.Action {
                text: "Show All Conversations"
                iconName: "view-list-details"
                checkable: true
                checked: true
                enabled: false
                onTriggered: {
                    print("Action checked:" + checked)
                }
                Controls.ButtonGroup.group: filterOptions
            },
            Kirigami.Action {
                text: "Show Only SMS"
                iconName: "view-list-details"
                checkable: true
                checked: false
                enabled: false
                onTriggered: {
                    print("Action checked:" + checked)
                }
                Controls.ButtonGroup.group: filterOptions
            },
            Kirigami.Action {
                text: "Show Only IM Chat"
                iconName: "view-list-details"
                checkable: true
                checked: false
                enabled: false
                onTriggered: {
                    print("Action checked:" + checked)
                }
                Controls.ButtonGroup.group: filterOptions
            }
        ]

        Controls.ButtonGroup { id: filterOptions }

//         RadioButton {
//             Controls.ButtonGroup.group: filterOptions
//             checked: true
//             text: "All Conversations"
//             enabled: false
//         }
//         RadioButton {
//             Controls.ButtonGroup.group: filterOptions
//             text: "Only SMS"
//             enabled: false
//         }
//         RadioButton {
//             Controls.ButtonGroup.group: filterOptions
//             text: "Only IM Chat"
//             enabled: false
//         }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent

//     Component {
//         id: settingsComponent
//         Kirigami.Page {
//             objectName: "settingsPage"
//             Rectangle {
//                 anchors.fill: parent
//             }
//         }
//     }

    //Main app content
    Component {
        id: mainPageComponent

        MainPage {}
    }

    Component {
        id: conversationPageComponent

        ConversationPage {}
    }

    Component {
        id: newConversationPageComponent

        NewConversationPage {
            id: newConversationPage
        }
    }

    Component.onCompleted: {
        telepathyManager.addTextChatFeatures();
        telepathyManager.becomeReady();
    }
}
