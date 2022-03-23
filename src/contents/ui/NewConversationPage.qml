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
    padding: 0

    ContactsList {
        anchors.fill: parent
        multiSelect: true
        showSections: true
        showNumber: true
        onClicked: {
            pageStack.pop()
            ChatListModel.startChat(Utils.phoneNumberList(numbers))
        }
    }
}
