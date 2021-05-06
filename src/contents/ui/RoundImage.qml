// SPDX-FileCopyrightText: 2017-2019 Kaidan Developers and Contributors
// SPDX-FileCopyrightText: 2019 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.6
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.6 as Kirigami

Kirigami.Icon {
    id: img
    property bool isRound: true

    layer.enabled: isRound
    layer.effect: OpacityMask {
        maskSource: Item {
            width: img.width
            height: img.height

            Rectangle {
                anchors.centerIn: parent
                width: Math.min(img.width, img.height)
                height: width
                radius: Math.min(width, height)
            }
        }
    }
}
