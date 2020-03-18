import QtQuick 2.1
import org.kde.kirigami 2.4 as Kirigami
import QtQuick.Controls 2.0 as Controls

import org.kde.spacebear 1.0 as Spacebear

Kirigami.ApplicationWindow {
    id: root

    title: i18n("spacebear")

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: "qrc:/ChatsPage.qml"
}
