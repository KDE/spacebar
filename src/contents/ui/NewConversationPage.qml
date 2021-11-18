// SPDX-FileCopyrightText:: Copyright 2015 Martin Klapetek <mklapetek@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    title: i18n("Contacts")

    property var selected: []

    function modifySelection(number, name) {
        const index = selected.findIndex(o => o.phoneNumber == number)
        if (index == -1) {
            selected.push({name: name, phoneNumber: number})
        } else {
            selected.splice(index, 1)
        }
        selected = selected
    }

    header: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true
            height: compose.height

            Kirigami.Heading {
                padding: Kirigami.Units.largeSpacing
                level: 4
                type: Kirigami.Heading.Type.Normal
                text: selected.length + " " + i18n("Selected")
                color: Kirigami.Theme.disabledTextColor
            }

            Row {
                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                padding: Kirigami.Units.smallSpacing
                Controls.Button {
                    id: compose
                    text: i18n("Compose")
                    onClicked: {
                        pageStack.pop()
                        ChatListModel.startChat(Utils.phoneNumberList(selected.map(o => o.phoneNumber)))
                    }
                }
            }
        }

        Controls.Control {
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing
            topPadding: 0

            contentItem: Kirigami.ActionTextField {
                id: searchField
                onTextChanged: contactsProxyModel.setFilterFixedString(text)
                inputMethodHints: Qt.ImhNoPredictiveText
                placeholderText: i18n("Search or enter number…")
                focusSequence: "Ctrl+F"
                rightActions: [
                    // Code copy from kirigami, existing actions are being overridden when setting the property
                    Kirigami.Action {
                        icon.name: searchField.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                        visible: searchField.text.length > 0 && !Utils.isPhoneNumber(searchField.text)
                        onTriggered: {
                            searchField.text = ""
                            searchField.accepted()
                        }
                    },
                    Kirigami.Action {
                        icon.name: "list-add-symbolic"
                        visible: searchField.text.length > 0 && Utils.isPhoneNumber(searchField.text)
                        onTriggered: {
                            // close new conversation page
                            pageStack.pop()
                            ChatListModel.startChat(Utils.phoneNumberList(searchField.text))
                            searchField.text = ""
                        }
                    }
                ]
            }
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: i18n("No contacts with phone numbers yet")
        visible: contactsProxyModel.rowCount() === 0
        helpfulAction: Kirigami.Action {
            text: i18n("Open contacts app")
            onTriggered: Utils.launchPhonebook()
        }
    }

    ListView {
        id: contactsList

        section.property: "display"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: Kirigami.ListSectionHeader {
            text: section
        }
        clip: true

        model: ContactModel {
            id: contactsProxyModel
        }

        reuseItems: true

        currentIndex: -1

        delegate: Kirigami.AbstractListItem {
            id: contact
            width: contactsList.width

            property string number: Utils.phoneNumberToInternationalString(Utils.phoneNumber(model.phoneNumber))

            contentItem: RowLayout {
                Controls.CheckBox {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    checked: selected.findIndex(o => o.phoneNumber == contact.number) != -1
                    checkable: false
                    onClicked: modifySelection(contact.number, model.name)
                }

                Kirigami.Avatar {
                    Layout.fillHeight: true
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    source: model.phoneNumber ? "image://avatar/" + contact.number : ""
                    name: model.display
                    imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
                }

                Kirigami.Heading {
                    level: 3
                    text: model.display
                    Layout.fillWidth: true
                }
            }
            onClicked: modifySelection(contact.number, model.name)
        }
    }
}
