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
    property string filterRegExp
    property bool executeDefaultAction: false

    signal contactClicked(string personUri)

    section.property: "display"
    section.criteria: ViewSection.FirstCharacter
    clip: true
    focus: true

    model: PlasmaCore.SortFilterModel {
        id: plasmaSortFilterModel
        sourceModel: KPeople.PersonsSortFilterProxyModel {
            id: kpeopleProxyModel
            sourceModel: contactsList.sourceModel
        }
        sortRole: "display"
        filterRole: "display"
        filterRegExp: ".*" + contactsList.filterRegExp + ".*"
        sortOrder: Qt.AscendingOrder
    }

    boundsBehavior: Flickable.StopAtBounds
//     highlightRangeMode: ListView.ApplyRange
//     highlight: PlasmaComponents.Highlight {
//
//     }
    highlightMoveDuration: 0

    KPeople.PersonActions {
        id: personActionsModel
    }

    onCurrentIndexChanged: print("---> " + currentIndex);

    delegate: Kirigami.AbstractListItem {
        supportsMouseEvents: true
        height: actionsRow.visible ? units.gridUnit * 6 : units.gridUnit * 3
        enabled: true
        clip: true
        opacity: contactsList.delegateSelected && contactsList.currentIndex != index ? 0.4 : 1

        onClicked: {
            contactsList.currentIndex = index;
            personActionsModel.personUri = model.personUri;
            if (contactsList.executeDefaultAction) {
                personActionsModel.triggerAction(0);
            } else {
                actionsListProxy.sourceModel = personActionsModel;
            }

            contactsList.contactClicked(model.personUri);
        }

        Item {
            anchors.fill: parent

            // Clear the actions model when index is switched
            Connections {
                target: contactsList
                onCurrentIndexChanged: {
                    if (contactsList.currentIndex != index) {
                        actionsListProxy.sourceModel = null;
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent

                RowLayout {
                    id: mainLayout
                    Layout.fillHeight: true
                    Layout.maximumHeight: units.gridUnit * 3
                    Layout.fillWidth: true

                    ExtraComponents.QPixmapItem {
                        id: avatarLabel

                        Layout.maximumWidth: parent.height
                        Layout.minimumWidth: parent.height
                        Layout.fillHeight: true

                        pixmap: model.decoration
                        fillMode: ExtraComponents.QPixmapItem.PreserveAspectFit
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

                RowLayout {
                    id: actionsRow
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    visible: actionsList.count > 0

                    ListView {
                        id: actionsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        orientation: ListView.Horizontal

                        model: PlasmaCore.SortFilterModel {
                            id: actionsListProxy
                            filterRole: "actionType"
                            filterCallback: function(source_row, value) { return value == KPeople.ActionType.TextChatAction; }
                        }

                        delegate: Controls.Button {
                            Layout.fillWidth: true
                            text: model.display
                            icon.source: model.iconName

                            onClicked: {
                                personActionsModel.triggerAction(actionsListProxy.mapRowToSource(index));
                            }
                        }
                    }
                }
            }
        }
    }

//                 CustomSectionScroller {
//                     listView: contactsList
//                 }

}
