// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: msgPage

    title: people.map(o => o.name || o.phoneNumber).join(",  ")

    property MessageModel messageModel;
    property real pointSize: Kirigami.Theme.defaultFont.pointSize + SettingsManager.messageFontSize
    property bool isNew: true
    property var people: messageModel ? messageModel.people : []

    Connections {
        target: pageStack
        function onCurrentItemChanged () {
            if (!pageStack.currentItem.hasOwnProperty("messageModel")) {
                messageModel.disableNotifications(Utils.phoneNumberList(""))
            }
        }
    }

    function getContrastYIQColor(hexcolor){
        hexcolor = hexcolor.replace("#", "");
        const r = parseInt(hexcolor.substr(0, 2), 16);
        const g = parseInt(hexcolor.substr(2, 2), 16);
        const b = parseInt(hexcolor.substr(4, 2), 16);
        const yiq = ((r * 299) + (g * 587) + (b * 114)) / 1000;

        return (yiq >= 128) ? Qt.rgba(0, 0, 0, 0.9) : Qt.rgba(255, 255, 255, 0.9);
    }

    function tryAddRecipient(number) {
        number = Utils.phoneNumberToInternationalString(Utils.phoneNumber(number))
        const index = people.findIndex(o => o.phoneNumber == number)

        if (index == -1) {
            people.push({ phoneNumber: number })
            if (pageStack.depth > 2) {
                pageStack.pop()
            }
            ChatListModel.startChat(Utils.phoneNumberList(people.map(o => o.phoneNumber)))
        } else {
            duplicateNotify.visible = true
            setTimeout(function () {
                duplicateNotify.visible = false
            }, 3000)
        }
    }

    function setTimeout(cb, delayTime) {
        timer.interval = delayTime
        timer.repeat = false
        timer.triggered.connect(cb)
        timer.start()
    }

    Timer {
        id: timer
    }

    actions {
        contextualActions: [
            Kirigami.Action {
                visible: !isNew
                iconName: "contact-new-symbolic"
                text: i18n("Add/remove")
                onTriggered: {
                    isNew = true
                }
            }
        ]
    }

    header: ColumnLayout {
        id: header

        Kirigami.InlineMessage {
            id: premiumWarning
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            type: Kirigami.MessageType.Warning
            text: i18n("Texting this premium SMS number might cause you to be charged money")
            visible: messageModel && Utils.isPremiumNumber(messageModel.phoneNumberList)
        }

        Flow {
            visible: people.length > 0 && isNew
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            spacing: Kirigami.Units.largeSpacing

            Repeater {
                model: people
                Rectangle {
                    color: "lightgrey"
                    radius: remove.height / 2
                    width: contact.width + remove.width
                    height: remove.height

                    Controls.RoundButton {
                        id: contact
                        height: parent.height
                        text: modelData.name || modelData.phoneNumber
                        flat: true
                        onClicked: Utils.launchPhonebook()
                    }

                    Controls.RoundButton {
                        id: remove
                        flat: true
                        icon.name: "edit-delete-remove"
                        anchors.right: parent.right
                        onClicked: {
                            const number = Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.phoneNumber))
                            const index = people.findIndex(o => o.phoneNumber == number)

                            people.splice(index, 1)
                            ChatListModel.startChat(Utils.phoneNumberList(people.map(o => o.phoneNumber)))
                        }
                    }
                }
            }
        }

        Controls.Control {
            visible: isNew
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.topMargin: 0

            contentItem: Kirigami.ActionTextField {
                id: searchField
                onTextChanged: {
                    contactsProxyModel.setFilterFixedString(text)
                    if (text.length == 1){
                        contactsList.open()
                    } else if (text.length == 0) {
                        contactsList.close()
                    }
                }
                onPressed: if (text.length > 0) contactsList.open()
                inputMethodHints: Qt.ImhNoPredictiveText
                placeholderText: i18n("Recipient")
                focusSequence: "Ctrl+F"
                rightActions: [
                    // Code copy from kirigami, existing actions are being overridden when setting the property
                    Kirigami.Action {
                        icon.name: "edit-delete-remove"
                        visible: searchField.text.length > 0 && !Utils.isPhoneNumber(searchField.text)
                        onTriggered: {
                            searchField.text = ""
                            searchField.accepted()
                            contactsList.close()
                        }
                    },
                    Kirigami.Action {
                        icon.name: "contact-new-symbolic"
                        visible: searchField.text.length == 0
                        onTriggered: {
                            pageStack.push("qrc:/NewConversationPage.qml", { selected: people })
                        }
                    },
                    Kirigami.Action {
                        icon.name: "list-add-symbolic"
                        visible: searchField.text.length > 0 && Utils.isPhoneNumber(searchField.text)
                        onTriggered: tryAddRecipient(searchField.text)
                    }
                ]
            }
        }
    }

    Controls.Popup {
        id: contactsList
        anchors.centerIn: parent
        topMargin: header.height + searchField.height
        padding: 0
        width: parent.width - Kirigami.Units.largeSpacing * 2
        height: parent.height - header.height - footer.height
        background: null

        contentItem: ListView {
            anchors.fill: parent

            clip: true

            model: ContactModel {
                id: contactsProxyModel
            }

            reuseItems: true

            currentIndex: -1

            delegate: Kirigami.AbstractListItem {
                width: parent.width || 0
                backgroundColor: Kirigami.Theme.backgroundColor

                contentItem: RowLayout {
                    Kirigami.Avatar {
                        Layout.fillHeight: true
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        source: contactsList.visible && model.phoneNumber ? "image://avatar/" + model.phoneNumber : ""
                        name: model.display
                        imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
                    }

                    Kirigami.Heading {
                        level: 3
                        text: model.display
                        Layout.fillWidth: true
                    }
                }
                onClicked: tryAddRecipient(model.phoneNumber)
            }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            onClicked: contactsList.close()
        }
    }

    ListView {
        id: listView
        model: messageModel
        spacing: Kirigami.Units.largeSpacing
        currentIndex: -1

        // configure chat bubble colors
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        property string incomingColor: SettingsManager.customMessageColors ? SettingsManager.incomingMessageColor : Kirigami.Theme.backgroundColor
        property string outgoingColor: SettingsManager.customMessageColors ? SettingsManager.outgoingMessageColor : Kirigami.Theme.highlightColor

        // adjust text for highlight color
        property string incomingTextColor: getContrastYIQColor(incomingColor)
        property string outgoingTextColor: getContrastYIQColor(outgoingColor)

        // when there is a new message or the the chat is first viewed, go to the bottom
        onCountChanged: delayCountChanged.restart()

        // delay timer to make sure listview gets positioned at the end
        Timer {
            id: delayCountChanged
            interval: 1
            repeat: false
            onTriggered: {
                if (listView.currentIndex > -1 && listView.currentIndex < listView.count) {
                    listView.positionViewAtIndex(listView.currentIndex, ListView.Visible)
                    listView.currentIndex = -1
                } else {
                    Qt.callLater( listView.positionViewAtEnd )
                }
            }
        }

        Component.onCompleted: Qt.callLater( listView.positionViewAtEnd )

        add: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }
        addDisplaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }

        section.property: "date"
        section.delegate: Controls.Control {
            padding: Kirigami.Units.largeSpacing
            width: parent.width
            contentItem:
            RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    height: 1
                    Layout.fillWidth: true
                }

                Text {
                    text: Qt.formatDate(section, Qt.locale().dateFormat(Locale.LongFormat))
                    horizontalAlignment: Text.AlignHCenter
                    font: Kirigami.Theme.smallFont
                    color: Kirigami.Theme.disabledTextColor
                }

                Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    height: 1
                    Layout.fillWidth: true
                }
            }
        }

        // remove focus from message entry field
        MouseArea {
            anchors.fill: parent
            z: -1
            onClicked: forceActiveFocus()
        }

        delegate: Item {
            id: delegateParent
            width: listView.width
            height: rect.height

            Component.onCompleted: {
                // Avoid unnecessary invocations

                // This code is only for marking messages as read that arrived after opening the chat.
                // However we currently don't know the id of those messages.
                // For now we use the function to mark all messages as read, but this should actually use
                // messageModel.markMessageRead(model.id)

                if (visible && !model.sentByMe && !model.read) {
                    Qt.callLater(ChatListModel.markChatAsRead, messageModel.phoneNumber);
                }
            }

            Kirigami.ShadowedRectangle {
                id: rect

                Kirigami.Theme.colorSet: Kirigami.Theme.Button
                
                anchors.margins: Kirigami.Units.largeSpacing
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined
                
                radius: Kirigami.Units.gridUnit
                corners.bottomRightRadius: model.sentByMe ? 0 : -1
                corners.topLeftRadius: model.sentByMe ? -1 : 0
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: Kirigami.Units.devicePixelRatio
                color: model.sentByMe ? listView.outgoingColor : listView.incomingColor
                height: content.height + Kirigami.Units.largeSpacing * 2
                width: content.width + Kirigami.Units.largeSpacing * 3

                ColumnLayout {
                    spacing: 0
                    id: content
                    anchors.centerIn: parent

                    property color textColor: model.sentByMe ? listView.outgoingTextColor : listView.incomingTextColor

                    // message contents
                    Controls.Label {
                        Layout.alignment: model.text && model.text.length > 1 ? Qt.AlignTop : Qt.AlignHCenter
                        Layout.minimumWidth: Kirigami.Units.gridUnit / 5
                        Layout.maximumWidth: delegateParent.width * 0.7
                        text: model.text ? model.text : " " // guarantee there is text so that height is maintained
                        wrapMode: Text.Wrap
                        textFormat: Text.StyledText
                        linkColor: model.sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.linkColor
                        color: content.textColor
                        font.pointSize: pointSize
                    }
                }

                Controls.Label {
                    anchors.left: model.sentByMe ? undefined : parent.right
                    anchors.right: model.sentByMe ? parent.left : undefined
                    anchors.bottom: parent.bottom
                    padding: Kirigami.Units.smallSpacing
                    bottomPadding: 0
                    text: Qt.formatTime(model.time, Qt.DefaultLocaleShortDate)
                    font: Kirigami.Theme.smallFont
                    color: Kirigami.Theme.disabledTextColor

                    Kirigami.Icon {
                        anchors.right: parent.left
                        anchors.bottom: parent.bottom
                        implicitHeight: Math.round(Kirigami.Units.gridUnit * 0.7)
                        implicitWidth: implicitHeight
                        source: {
                            if (visible) {
                                switch (model.deliveryState) {
                                    case MessageModel.Unknown:
                                        return "dontknow";
                                    case MessageModel.Pending:
                                        return "content-loading-symbolic";
                                    case MessageModel.Sent:
                                        return "answer-correct";
                                    case MessageModel.Failed:
                                        return "error"
                                }
                            }
                            
                            return undefined
                        }
                        
                        visible: !!(model.sentByMe && (model.deliveryState != MessageModel.Sent || model.index == (listView.count - 1)))
                        color: model.deliveryState == MessageModel.Failed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.disabledTextColor
                    }
                }
            }

            MouseArea {
                anchors.fill: rect
                onPressAndHold: {
                    menu.index = index
                    menu.id = model.id
                    menu.text = model.text
                    menu.open()
                }
                onPressed: parent.forceActiveFocus()
            }
        }
    }

    Kirigami.OverlayDrawer {
        id: menu

        property int index
        property string id
        property string text

        edge: Qt.BottomEdge

        contentItem: ColumnLayout {
            Kirigami.BasicListItem {
                visible: menu.text.match(/[0-9]{6}/)
                text: i18n("Copy code")
                icon: "edit-copy"
                onClicked: {
                    Utils.copyTextToClipboard(menu.text.match(/[0-9]{6}/))
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                visible: menu.text.indexOf('href="') >= 0
                text: i18n("Copy link")
                icon: "edit-copy"
                onClicked: {
                    const start = menu.text.indexOf('href="')
                    const finish = menu.text.indexOf('"', start + 6)
                    let link = menu.text.substring(start + 6, finish)
                    Utils.copyTextToClipboard(link)
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                text: i18n("Copy text")
                icon: "edit-copy"
                onClicked: {
                    Utils.copyTextToClipboard(menu.text)
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                text: i18n("Delete message")
                icon: "edit-delete"
                onClicked: {
                    listView.currentIndex = menu.index
                    messageModel.deleteMessage(menu.id, menu.index)
                    menu.close()
                }
            }
        }
    }

    footer: Kirigami.ActionTextField {
        id: field
        height: Kirigami.Units.gridUnit * 2
        placeholderText: {
            var number = Utils.sendingNumber()
            if (number === "0") {
                return i18n("Write Message...")
            } else {
                return i18nc("%1 is a phone number", "Send Message from %1...", number)
            }
        }
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
        font.pointSize: pointSize

        Kirigami.InlineMessage {
            id: duplicateNotify
            width: parent.width
            type: Kirigami.MessageType.Warning
            text: i18n("Duplicate recipient")
            visible: false
        }
    }
}
