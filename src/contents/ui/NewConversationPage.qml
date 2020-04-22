// SPDX-FileCopyrightText:: Copyright 2015 Martin Klapetek <mklapetek@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.7
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.10 as Kirigami
import org.kde.people 1.0 as KPeople
import org.kde.spacebear 1.0

Kirigami.ScrollablePage {
    title: i18n("Contacts")

    header: Kirigami.ActionTextField {
        id: searchField
        onTextChanged: contactsProxyModel.setFilterFixedString(text)
        placeholderText: i18n("Search or enter number…")
        focusSequence: "Ctrl+F"
        rightActions: [
            Kirigami.Action {
                icon.name: searchField.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                visible: searchField.text.length > 0 && !Utils.isPhoneNumber(searchField.text)
                onTriggered: {
                    searchField.text = ""
                    searchField.accepted()
                }
            },
            Kirigami.Action {
                icon.name: "document-send"
                visible: searchField.text.length > 0 && Utils.isPhoneNumber(searchField.text)
                onTriggered: {
                    searchField.text = ""

                    // close new conversation page
                    pageStack.pop()

                    ChatListModel.startChat(searchField.text)
                }
            }
        ]
    }

    Controls.Label {
        anchors.centerIn: parent
        text: i18n("No contacts")
        visible: contactsList.count === 0
    }

    ListView {
        id: contactsList

        anchors.fill: parent

        section.property: "display"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: Kirigami.ListSectionHeader {
            text: section
        }
        clip: true

        model: KPeople.PersonsSortFilterProxyModel {
            id: contactsProxyModel
            sourceModel: KPeople.PersonsModel {
                id: contactsModel
            }
            requiredProperties: "phoneNumber"
            filterRole: Qt.DisplayRole
            sortRole: Qt.DisplayRole
            filterCaseSensitivity: Qt.CaseInsensitive
            Component.onCompleted: sort(0)
        }

        boundsBehavior: Flickable.StopAtBounds

        Component {
            id: contactListDelegate

            Kirigami.BasicListItem {
                icon: model && model.decoration
                label: model && model.display
                onClicked: {
                    ChatListModel.startChat(model.phoneNumber)
                    pageStack.pop()
                }
            }
        }

        delegate: Kirigami.DelegateRecycler {
            width: parent ? parent.width : 0
            sourceComponent: contactListDelegate
        }
    }
}
