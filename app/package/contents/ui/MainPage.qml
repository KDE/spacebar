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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.spacebar 1.0

ColumnLayout {
    id: rootLayout
    anchors.fill: parent

    ListView {
        Layout.fillHeight: true
        Layout.fillWidth: true

        model: MainLogModel {
        }

        delegate: PlasmaComponents.ListItem {

            onClicked: {
                root.pageStack.push(root.conversationPageComponent);
            }

            ColumnLayout {
                PlasmaComponents.Label {
                    text: model.lastMessageDate
                }
                PlasmaComponents.Label {
                    text: model.contactIdRole
                }

                PlasmaComponents.Label {
                    text: model.lastMessageText
                }
            }
        }
    }
}
