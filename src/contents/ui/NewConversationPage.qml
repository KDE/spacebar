// SPDX-FileCopyrightText:: Copyright 2015 Martin Klapetek <mklapetek@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.14 as Kirigami
import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    title: i18n("Contacts")

    header: Controls.Control {
        padding: Kirigami.Units.largeSpacing

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
                    icon.name: "document-send"
                    visible: searchField.text.length > 0 && Utils.isPhoneNumber(searchField.text)
                    onTriggered: {
                        // close new conversation page
                        pageStack.pop()

                        ChatListModel.startChat(searchField.text)

                        searchField.text = ""
                    }
                }
            ]
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
            width: contactsList.width
            contentItem: RowLayout {
                Kirigami.Avatar {
                    id: photo
                    Layout.fillHeight: true
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium

                    source: "image://avatar/" + (model && model.phoneNumber)
                    name: model && model.display
                    imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
                }

                Kirigami.Heading {
                    level: 3
                    text: model && model.display
                    Layout.fillWidth: true
                }
            }
            onClicked: {
                pageStack.pop()
                ChatListModel.startChat(model.phoneNumber)
            }
        }
    }
}
