/***************************************************************************
 *   Copyright (C) 2016 by Martin Klapetek <mklapetek@kde.org>             *
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
import QtQuick.Layouts 1.1
import org.kde.telepathy 0.1 as KTp
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Flickable {
    Layout.minimumWidth: units.gridUnit * 10
    Layout.minimumHeight: units.gridUnit * 20

    ListView {
        anchors.fill: parent
        model: presenceModel

        delegate: PlasmaComponents.ListItem {
            id: listItem
            checked: presence === ktpPresence.currentPresence
            enabled: true

            onClicked: {
                ktpPresence.requestedPresence = presence;
            }

            RowLayout {

                PlasmaCore.IconItem {
                    Layout.maximumHeight: units.iconSizes.medium
                    source: decoration

                    PlasmaComponents.BusyIndicator {
                        anchors.fill: parent
                        visible: presence === ktpPresence.requestedPresence && !listItem.checked
                        running: visible
                    }
                }

                PlasmaComponents.Label {
                    Layout.fillWidth: true

                    text: display
                }
            }
        }
    }
}
