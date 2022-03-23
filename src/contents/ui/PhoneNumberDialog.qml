/*
 *   SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.19 as Kirigami

import org.kde.spacebar 1.0

Kirigami.Dialog {
    id: root

    property alias numbers: list.model

    signal numberSelected(string number)

    ListView {
        id: list
        implicitWidth: Kirigami.Units.gridUnit * 20
        implicitHeight: contentHeight
        currentIndex: -1

        delegate: Kirigami.BasicListItem {
            text: modelData.typeLabel
            subtitle: Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.number))
            onClicked: {
                close()
                root.numberSelected(modelData.normalizedNumber)
            }
        }
    }
}
