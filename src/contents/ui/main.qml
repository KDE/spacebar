// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Controls as Controls

import org.kde.spacebar as Spacebar

Kirigami.ApplicationWindow {
    id: root

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack {
        initialPage: "qrc:/ChatsPage.qml"
        globalToolBar {
            canContainHandles: true
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: Kirigami.ApplicationHeaderStyle.ShowBackButton
        }
    }
}
