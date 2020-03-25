import QtQuick 2.0
import QtQuick.Controls 2.0 as Controls

import org.kde.kirigami 2.8 as Kirigami

import org.kde.spacebear 1.0

Kirigami.ScrollablePage {
    title: messageModel.person.name || messageModel.person.phoneNumber || messageModel.phoneNumber
    property MessageModel messageModel;

    Connections {
        target: messageModel
    }

    Controls.BusyIndicator {
        anchors.centerIn: parent
        running: !messageModel.isReady
        visible: !messageModel.isReady
    }

    ListView {
        model: messageModel
        delegate: Kirigami.BasicListItem {
            text: model.text
        }
    }

    footer: Kirigami.ActionTextField {
        id: field
        height: 40
        placeholderText: i18n("Write Message...")
        onAccepted: sendAction.triggered()
        rightActions: [
            Kirigami.Action {
                id: sendAction
                text: i18n("Send")
                icon.name: "document-send"
                enabled: field.text !== ""
                onTriggered: {
                    messageModel.sendMessage(field.text)
                    field.text = ""
                }
            }
        ]
    }
}
