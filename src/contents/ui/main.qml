// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami
import QtQuick.Controls 2.0 as Controls

import org.kde.spacebar 1.0 as Spacebar

Kirigami.ApplicationWindow {
    id: root

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.globalToolBar.canContainHandles: true
    pageStack.initialPage: "qrc:/ChatsPage.qml"
}
