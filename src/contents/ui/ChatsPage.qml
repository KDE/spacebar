import QtQuick 2.0
import org.kde.kirigami 2.2 as Kirigami
import QtQuick.Controls 2.4 as Controls

import org.kde.spacebear 1.0 as Spacebear

Kirigami.ScrollablePage {
    title: i18n("Chats")

    ListView {
        model: Spacebear.ChatListModel {id: chatModel}

        delegate: Kirigami.BasicListItem {
            text: model.displayName || model.phoneNumber
            onClicked: chatModel.startChat(model.phoneNumber)
        }
    }
}
