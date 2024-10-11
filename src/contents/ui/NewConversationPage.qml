// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.spacebar

Kirigami.ScrollablePage {
    id: root
    title: list.multiSelect ? i18n("New Group Chat") : i18n("New Chat")
    padding: 0

    property var selected: []

    NewConversationContactsList {
        id: list
        anchors.fill: parent
        showSections: true
        showNumber: true

        multiSelect: selected.length > 0

        onRequestCreateChat: (numbers) => {
            while (pageStack.depth > 1) {
                pageStack.pop()
            }

            ChatListModel.startChat(Utils.phoneNumberList(numbers))
        }
    }

    Component.onCompleted: list.selected = selected
}
