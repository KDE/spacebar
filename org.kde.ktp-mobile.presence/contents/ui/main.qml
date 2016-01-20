/***************************************************************************
 *   Copyright (C) 2011 by Francesco Nwokeka <francesco.nwokeka@gmail.com> *
 *   Copyright (C) 2014 by Aleix Pol Gonzalez <aleixpol@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

import QtQuick 2.1
import org.kde.telepathy 0.1 as KTp
import org.kde.plasma.plasmoid 2.0

Item
{
    id: root

    Plasmoid.switchWidth: 200
    Plasmoid.switchHeight: 300

    Plasmoid.fullRepresentation: PresenceList {}
    Plasmoid.compactRepresentation: Presence {
        source: ktpPresence.currentPresenceIconName
    }

    Plasmoid.busy: ktpPresence.isChangingPresence

    KTp.PresenceModel {
        id: presenceModel
    }

    KTp.GlobalPresence {
        id: ktpPresence
        accountManager: telepathyManager.accountManager

        onRequestedPresenceChanged: {
            updateTooltip();
        }

        onCurrentPresenceChanged: {
            updateTooltip();
        }

        onConnectionStatusChanged: {
            updateTooltip();
        }
    }

    function setPresence(row) {
        ktpPresence.requestedPresence = presenceModel.get(row, "presence");

    }

    function updateTooltip() {
        if (ktpPresence.isChangingPresence) {
            if (ktpPresence.presenceType == KTp.GlobalPresence.Offline) {
                Plasmoid.toolTipSubText = i18nc("Means 'Connecting your IM accounts', it's in the applet tooltip", "Connecting...");
            } else {
                Plasmoid.toolTipSubText = i18nc("The arg is the presence name (as is in ktp-common-internals.po, eg. Changing Presence to Away..., it's in the applet tooltip",
                                                "Changing Presence to %1...", ktpPresence.requestedPresenceName);
            }
        } else {
            Plasmoid.toolTipSubText = ktpPresence.currentPresenceName;
        }
    }

    Component.onCompleted: {
        telepathyManager.addContactListFeatures();
        telepathyManager.becomeReady();

        //TODO: The PresenceModel might change, this will never react to such changes
        for(var i=0; i<presenceModel.count; ++i) {
            var disp = presenceModel.get(i, "display");
            var actionName = i;
            plasmoid.setAction(actionName, disp, presenceModel.get(i, "iconName"));
        }
        plasmoid.setActionSeparator("statuses");

        // application actions
        plasmoid.setAction("openIMSettings", i18n("Instant Messaging Settings..."), "configure");
        plasmoid.setAction("openContactList", i18n("Contact List..."), "telepathy-kde");
        plasmoid.setActionSeparator("applications");

        plasmoid.setAction("addContact", i18n("Add New Contacts..."), "list-add-user");
        plasmoid.setAction("joinChatRoom", i18n("Join Chat Room..."), "im-irc");

        if (telepathyManager.canDial) {
            plasmoid.setAction("dial", i18n("Make a Call..."), "internet-telephony");
        }
        if (telepathyManager.canSendFile) {
            plasmoid.setAction("sendFile", i18n("Send a File..."), "mail-attachment");
        }
        plasmoid.setActionSeparator("actions");
    }

    function action_dial() { telepathyManager.openDialUi(); }
    function action_sendFile() { telepathyManager.openDialUi(); }
    function action_addContact() { telepathyManager.addContact(); }
    function action_joinChatRoom() { telepathyManager.joinChatRoom(); }
    function action_openContactList() { telepathyManager.toggleContactList(); }
    function action_openIMSettings() {telepathyManager.showSettingsKCM(); }

    function actionTriggered(actionName) {
        var number = parseInt(actionName)
        if (number !== null) {
            root.setPresence(number)
        } else {
            console.log("Unknown action", actionName);
        }
    }
}
