import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0 as Controls

import org.kde.kirigami 2.12 as Kirigami

import org.kde.spacebear 1.0

Kirigami.ScrollablePage {
    id: msgPage

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
        id: listView
        model: messageModel
        spacing: 10
        currentIndex: count - 1
        Timer {
            interval: 1
            repeat: false
            running: true

            onTriggered: listView.positionViewAtIndex(listView.count - 1, ListView.End)
        }

        delegate: Item {
            width: listView.width
            height: rect.height

            Kirigami.ShadowedRectangle {
                id: rect
                anchors.margins: 20
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined
                radius: 10
                shadow.size: 4
                color: Qt.lighter(Kirigami.Theme.visitedLinkBackgroundColor, 1.05)
                height: content.height + 10
                width: content.width + 10
                ColumnLayout {
                    id: content
                    anchors.centerIn: parent
                    Controls.Label {
                        Layout.alignment: Qt.AlignTop
                        text: model.text
                        wrapMode: Text.WordWrap
                        Layout.minimumWidth: 100
                        Layout.minimumHeight: 30
                        Layout.maximumWidth: msgPage.width * 0.7
                    }
                }
            }
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
