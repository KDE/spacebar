/*
 *   SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.spacebar

Kirigami.Dialog {
    id: root

    property alias numbers: list.model
    property var selected: []

    signal numberSelected(string number)

    ListView {
        id: list
        implicitWidth: Kirigami.Units.gridUnit * 20
        implicitHeight: contentHeight
        currentIndex: -1

        delegate: Delegates.RoundedItemDelegate {
            id: delegateItem
            width: parent.width
            implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
            verticalPadding: 0
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                RowLayout {
                    Controls.CheckBox {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height
                        checked: selected.findIndex(o => o.phoneNumber == subtitleItem.text) >= 0
                        checkable: true
                        onToggled: root.numberSelected(modelData.normalizedNumber)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Controls.Label {
                        id: labelItem
                        Layout.fillWidth: true
                        Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
                        text: modelData.typeLabel
                        elide: Text.ElideRight
                        color: Kirigami.Theme.textColor
                    }
                    Controls.Label {
                        id: subtitleItem
                        visible: text
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        text: Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.number))
                        elide: Text.ElideRight
                        color: Kirigami.Theme.textColor
                        opacity: 0.7
                        font: Kirigami.Theme.smallFont
                    }
                }

            }
            onClicked: {
                close()
                root.numberSelected(modelData.normalizedNumber)
            }
        }
    }
}
