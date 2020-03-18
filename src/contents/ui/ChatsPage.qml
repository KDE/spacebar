import QtQuick 2.0
import org.kde.kirigami 2.2 as Kirigami
import QtQuick.Controls 2.4 as Controls

import org.kde.spacebear 1.0 as Spacebear

Kirigami.ScrollablePage {
    title: i18n("Chats")

    actions {
        main: Kirigami.Action {
            text: i18n("New Conversation")
            onTriggered: pageStack.push("qrc:/NewConversationPage.qml", {"chatModel": chatModel})
            icon.name: "contact-new"
        }
    }

    ListView {
        model: Spacebear.ChatListModel {
            id: chatModel
            onChatStarted: pageStack.push("qrc:/ChatPage.qml", {"messageModel": messageModel})
        }

        delegate: Kirigami.BasicListItem {
            height: Kirigami.Units.gridUnit * 2.5
            text: model.displayName || model.phoneNumber
            reserveSpaceForIcon: true
            icon: model.photo
            onClicked: chatModel.startChat(model.phoneNumber)
        }
    }
}
