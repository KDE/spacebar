// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.spacebar

Kirigami.ScrollablePage {
    title: i18n("Contacts")
    padding: 0

    property var selected: []

    ContactsList {
        id: list
        anchors.fill: parent
        multiSelect: true
        showSections: true
        showNumber: true
        onClicked: {
            while (pageStack.depth > 1) {
                pageStack.pop()
            }

            ChatListModel.startChat(Utils.phoneNumberList(numbers))
        }
    }

    Component.onCompleted: list.selected = selected
}
