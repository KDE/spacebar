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

MobileComponents.ApplicationWindow {
    id: root

    width: 800
    height: 1080

    actionButton.onClicked: {
        pageStack.pop(pageStack.initialPage);
        pageStack.push(conversationPageComponent);
        print("Action button clicked")
    }
    actionButton.iconSource: "document-edit"

    globalDrawer: MobileComponents.GlobalDrawer {
        title: "SpaceBar"
        titleIcon: "spacebar"
//         bannerImageSource: "banner.jpg"

        actions: [
//         MobileComponents.ActionGroup {
//             text: "View"
//             iconName: "view-list-icons"
//             Action {
//                 text: "action 1"
//             }
//             Action {
//                 text: "action 2"
//             }
//             Action {
//                 text: "action 3"
//             }
//         },
        Action {
            text: "Show All Conversations"
            iconName: "view-list-details"
            checkable: true
            checked: true
            enabled: false
            onTriggered: {
                print("Action checked:" + checked)
            }
            exclusiveGroup: filterOptions
        },
        Action {
            text: "Show Only SMS"
            iconName: "view-list-details"
            checkable: true
            checked: false
            enabled: false
            onTriggered: {
                print("Action checked:" + checked)
            }
            exclusiveGroup: filterOptions
        },
        Action {
            text: "Show Only IM Chat"
            iconName: "view-list-details"
            checkable: true
            checked: false
            enabled: false
            onTriggered: {
                print("Action checked:" + checked)
            }
            exclusiveGroup: filterOptions
        }
        ]

        ExclusiveGroup { id: filterOptions }

//         RadioButton {
//             exclusiveGroup: filterOptions
//             checked: true
//             text: "All Conversations"
//             enabled: false
//         }
//         RadioButton {
//             exclusiveGroup: filterOptions
//             text: "Only SMS"
//             enabled: false
//         }
//         RadioButton {
//             exclusiveGroup: filterOptions
//             text: "Only IM Chat"
//             enabled: false
//         }
    }
    contextDrawer: MobileComponents.ContextDrawer {
        id: contextDrawer
    }

    MobileComponents.OverlayDrawer {
        id: sheet
        edge: Qt.BottomEdge
        contentItem: Item {
            implicitWidth: MobileComponents.Units.gridUnit * 8
            implicitHeight: MobileComponents.Units.gridUnit * 8
            ColumnLayout {
                anchors.centerIn: parent
                Button {
                    text: "Button1"
                }
                Button {
                    text: "Button2"
                }
            }
        }
    }
    initialPage: mainPageComponent

//     Component {
//         id: settingsComponent
//         MobileComponents.Page {
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

    Component.onCompleted: {
        telepathyManager.addTextChatFeatures();
        telepathyManager.becomeReady();
    }
}
