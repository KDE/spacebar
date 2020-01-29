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
import QtQuick.Controls 2.3 as Controls
import QtQuick.Layouts 1.1
import org.kde.people 1.0 as KPeople
import org.kde.kquickcontrolsaddons 2.0 as ExtraComponents
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kirigami 2.1 as Kirigami
import org.kde.telepathy 0.1

ListView {
    id: contactsList

    property bool delegateSelected: false
    property string numberToCall

    property alias requiredProperties: kpeopleProxyModel.requiredProperties
    property QtObject sourceModel
    property string filterString
    property bool executeDefaultAction: false

    signal contactClicked(string personUri)

    onFilterStringChanged: kpeopleProxyModel.setFilterFixedString(filterString)

    section.property: "display"
    section.criteria: ViewSection.FirstCharacter
    clip: true
    focus: true

    model: KPeople.PersonsSortFilterProxyModel {
        id: kpeopleProxyModel
        sourceModel: contactsList.sourceModel
        filterRole: Qt.DisplayRole
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    highlightMoveDuration: 0

    KPeople.PersonActions {
        id: personActionsModel
    }

    onCurrentIndexChanged: print("---> " + currentIndex);

    delegate: Kirigami.AbstractListItem {
        supportsMouseEvents: true
        height: Kirigami.Units.gridUnit * 3
        enabled: true
        clip: true

        onClicked: contactsList.contactClicked(model.personUri)

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                id: mainLayout
                Layout.fillHeight: true
                Layout.maximumHeight: Kirigami.Units.gridUnit * 3
                Layout.fillWidth: true

                Kirigami.Icon {
                    id: avatarLabel

                    Layout.maximumWidth: parent.height
                    Layout.minimumWidth: parent.height
                    Layout.fillHeight: true

                    source: model.decoration
                    smooth: true
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Controls.Label {
                        id: nickLabel

                        Layout.fillWidth: true

                        text: model.display
                        elide: Text.ElideRight
                    }

                    Controls.Label {
                        id: dataLabel

                        Layout.fillWidth: true

                        text: model.phoneNumber !== undefined ? model.phoneNumber : (model.accountDisplayName !== undefined ? model.accountDisplayName : "")
                        elide: Text.ElideRight
                        visible: dataLabel.text !== nickLabel.text
                        opacity: 0.4
                    }

                }
            }
        }
    }
}
