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
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1
import org.kde.plasma.private.kpeoplehelper 1.0
import org.kde.kirigami 2.1 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.telepathy 0.1

Kirigami.ScrollablePage {
    focus: true

    title: i18n("Start New Conversation")
    property string pageName: "newConversationPage"

    header: Rectangle {
        clip: true
        id: header
        height: toInputField.implicitHeight + 2 * Kirigami.Units.largeSpacing
        width: root.width

        Controls.TextField {
            id: toInputField
            placeholderText: i18n("Search")
            anchors.centerIn: parent
            anchors.margins: Kirigami.Units.largeSpacing
            width: parent.width - 2 * Kirigami.Units.largeSpacing
        }
    }

    ContactList {
        anchors.fill: parent
        id: contactListView
        requiredProperties: ["phoneNumber", "telepathy-contactUri"]
        executeDefaultAction: true
        filterRegExp: toInputField.text
        sourceModel: KPeopleHelper {
            id: contactsModel
        }

        onContactClicked: {
            root.requestedChannel = personUri;
        }
    }
}
