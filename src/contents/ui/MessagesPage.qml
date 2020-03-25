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

        // HACK
        Timer {
            interval: 1
            repeat: false
            running: true

            onTriggered: listView.positionViewAtIndex(listView.count - 1, ListView.End)
        }

        section.property: "date"
        section.delegate: Item {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 50

            Rectangle {
                anchors.centerIn: parent
                id: dateRect
                color: "lightgrey"
                opacity: 0.2
                height: label.implicitHeight + 5
                width: label.implicitWidth + 10
                radius: height * 0.5
            }
            Controls.Label {
                id: label
                anchors.centerIn: dateRect
                text: Qt.formatDate(section, Qt.DefaultLocaleLongDate)
            }
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
                shadow.size: 3
                //Kirigami.Theme.colorSet: model.sentByMe ? Kirigami.Theme.Selection : Kirigami.Theme.View
                color: {
                    if (model.sentByMe) {
                        var col = Kirigami.Theme.highlightColor
                        col.a = 0.1
                        Qt.tint(Kirigami.Theme.visitedLinkBackgroundColor, col)
                    } else {
                        Qt.lighter(Kirigami.Theme.visitedLinkBackgroundColor, 1.04)
                    }
                }
                height: content.height + 10
                width: content.width + 10
                ColumnLayout {
                    spacing: 0
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
                    Controls.Label {
                        color: Kirigami.Theme.disabledTextColor
                        Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                        text: Qt.formatTime(model.time, Qt.DefaultLocaleShortDate)
                    }
                }
            }
        }
    }

    footer: Kirigami.ActionTextField {
        id: field
        height: 40
        placeholderText: i18n("Write Message...")
        onAccepted: text !== "" && sendAction.triggered()
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
