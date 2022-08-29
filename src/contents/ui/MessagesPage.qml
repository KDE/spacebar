// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls
import QtGraphicalEffects 1.15
import QtQuick.Dialogs 1.3

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    id: msgPage
    title: people.length === 0 ? i18n("New message") : people.map(o => o.name || o.phoneNumber).join(",  ")

    property MessageModel messageModel;
    property real pointSize: Kirigami.Theme.defaultFont.pointSize + SettingsManager.messageFontSize
    property bool isNew: true
    property var people: messageModel ? messageModel.people : []
    property string attachmentsFolder: messageModel ? messageModel.attachmentsFolder : "";
    property ListModel files: ListModel {}
    property var tapbackKeys: ["♥️", "👍" , "👎", "😂", "‼️", "❓"]

    Connections {
        target: pageStack
        function onCurrentItemChanged () {
            if (!pageStack.currentItem.hasOwnProperty("messageModel")) {
                messageModel.disableNotifications(Utils.phoneNumberList(""))
            }
        }
    }

    Component.onCompleted: {
        if (isNew) {
            listView.header = contactsList
        }
    }

    Component {
        id: contactsList

        ContactsList {
            visible: isNew
            z: 3
            width: listView.width
            height: listView.height
            multiSelect: false
            showSections: false
            showAll: false
            onSelect: {
                tryAddRecipient(number)
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

    function filesTotalSize() {
        let size = 0
        for (let i = 0; i < files.count; i++) {
            size += (files.get(i).size || 0)
        }

        return size
    }

    function formatBytes(bytes, decimals = 1) {
        if (bytes === 0) return '';

        const k = 1024;
        const sizes = ['B', 'KiB', 'MiB', 'GiB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));

        return parseFloat((bytes / Math.pow(k, i)).toFixed(decimals)) + ' ' + sizes[i];
    }

    function filesToList() {
        const list = [];
        for (let i = 0; i < files.count; i++) {
            list.push(files.get(i).filePath || files.get(i).text);
        }

        return list;
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

    function lookupSenderName(number) {
        if (people.length < 2) {
            return ""
        }
        const person = people.find(o => o.phoneNumber === number)
        return person.name || person.phoneNumber
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
                    listView.header = contactsList
                }
            }
        ]
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: premiumWarning
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.bottomMargin: 0
            type: Kirigami.MessageType.Warning
            text: i18n("Texting this premium SMS number might cause you to be charged money")
            visible: messageModel && Utils.isPremiumNumber(messageModel.phoneNumberList)
        }

        Kirigami.InlineMessage {
            id: maxAttachmentsError
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: i18n("Max attachment limit exceeded")
            visible: files.count > SettingsManager.maxAttachments
        }

        Kirigami.InlineMessage {
            id: mmscUrlMissingError
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: i18n("No MMSC configured")
            visible: SettingsManager.mmsc.length === 0 && (files.count > 0 || people.length > 1)
        }

        Kirigami.InlineMessage {
            id: messageExpiredError
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            text: i18n("Message has expired and will be deleted")
            visible: false
        }

        Kirigami.InlineMessage {
            id: messageGroupAsIndividual
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            text: i18n("Message will be sent as individual messages")
            visible: people.length > 1 && !SettingsManager.groupConversation
        }

        Flow {
            visible: people.length > 0 && isNew
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
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
    }

    ListView {
        id: listView
        model: messageModel
        spacing: Kirigami.Units.largeSpacing
        currentIndex: -1
        headerPositioning: ListView.OverlayHeader

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
            bottomPadding: Kirigami.Units.gridUnit * 1.5
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
            height: rect.height + senderDisplay.implicitHeight

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
                anchors.margins: Kirigami.Units.largeSpacing
                anchors.left: model.sentByMe ? undefined : parent.left
                anchors.right: model.sentByMe ? parent.right : undefined

                property int padding: Kirigami.Units.largeSpacing * 2
                property var tapbacks: model.tapbacks ? JSON.parse(model.tapbacks) : ""

                radius: Kirigami.Units.gridUnit
                corners.bottomRightRadius: model.sentByMe ? 0 : -1
                corners.topLeftRadius: model.sentByMe ? -1 : 0
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: 1
                color: model.sentByMe ? listView.outgoingColor : listView.incomingColor
                height: content.height + padding
                width: content.width + padding * 1.5

                MouseArea {
                    anchors.fill: parent
                    onClicked: if (!listView.moving && (content.attachments.length > 1 || modelText.truncated)) {
                        pageStack.layers.push("qrc:/FullscreenPage.qml", {
                            recipients: msgPage.title,
                            text: model.text,
                            attachments: content.attachments,
                            folder: attachmentsFolder
                        } )
                    }
                }

                property string sender: {
                    if (!model.sentByMe && model.fromNumber) {
                        return lookupSenderName(model.fromNumber)
                    }
                    return ""
                }

                Text {
                    id: senderDisplay
                    visible: rect.sender
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    leftPadding: Kirigami.Units.smallSpacing
                    text: rect.sender
                    font: Kirigami.Theme.smallFont
                    color: Kirigami.Theme.disabledTextColor
                }

                ColumnLayout {
                    id: content
                    spacing: Kirigami.Units.largeSpacing
                    anchors.centerIn: parent

                    property color textColor: model.sentByMe ? listView.outgoingTextColor : listView.incomingTextColor
                    property var attachments: model.attachments ? JSON.parse(model.attachments) : []
                    property bool clipped: false

                    // message contents
                    Controls.Label {
                        id: modelText
                        visible: !!model.text
                        Layout.alignment: model.text && model.text.length > 1 ? Qt.AlignTop : Qt.AlignHCenter
                        Layout.minimumWidth: Kirigami.Units.gridUnit / 5
                        Layout.maximumWidth: Math.round(delegateParent.width * 0.7)
                        maximumLineCount: 12
                        text: Utils.textToHtml(model.text)
                        wrapMode: Text.Wrap
                        textFormat: Text.StyledText
                        linkColor: model.sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.linkColor
                        color: content.textColor
                        font.pointSize: pointSize
                        font.family: "Noto Sans, Noto Color Emoji"
                        onLinkActivated: Qt.openUrlExternally(link)
                    }

                    // download message contents
                    RowLayout {
                        visible: model.pendingDownload
                        Layout.maximumWidth: Math.round(delegateParent.width * 0.7)

                        MouseArea {
                            enabled: parent.visible && model.deliveryState != MessageModel.Pending
                            width: parent.width
                            height: parent.height
                            onClicked: {
                                model.deliveryState = MessageModel.Pending

                                // check if expired
                                if (new Date(model.expires) < new Date()) {
                                    messageExpiredError.visible = true
                                    setTimeout(function () {
                                        listView.currentIndex = index
                                        messageModel.deleteMessage(model.id, index,[])
                                        messageExpiredError.visible = false
                                    }, 5000)
                                } else {
                                    messageModel.downloadMessage(model.id, model.contentLocation, model.expires)
                                }
                            }
                        }

                        Controls.BusyIndicator {
                            scale: pointSize / Kirigami.Theme.defaultFont.pointSize
                            running: model.deliveryState == MessageModel.Pending

                            Kirigami.Icon {
                                visible: !parent.running
                                anchors.fill: parent
                                source: model.deliveryState === MessageModel.Failed ? "state-error" : "folder-download-symbolic"
                                color: content.textColor
                            }
                        }

                        Column {
                            Kirigami.Heading {
                                text: model.subject || i18n("MMS message")
                                wrapMode: Text.Wrap
                                color: content.textColor
                                font.pointSize: pointSize
                                level: 3
                                type: Kirigami.Heading.Type.Primary
                            }
                            Controls.Label {
                                text: model.size ? i18n("Message size: %1", formatBytes(model.size)) : ""
                                wrapMode: Text.Wrap
                                color: content.textColor
                                font.pointSize: pointSize
                            }
                            Controls.Label {
                                text: i18n("Expires: %1", model.expiresDateTime)
                                wrapMode: Text.Wrap
                                color: content.textColor
                                font.pointSize: pointSize
                            }
                        }
                    }

                    // message attachments
                    Repeater {
                        model: content.attachments

                        Column {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: Kirigami.Units.smallSpacing
                            Layout.minimumWidth: Kirigami.Units.largeSpacing * 2
                            Layout.minimumHeight: Kirigami.Units.largeSpacing * 2

                            readonly property bool isImage: modelData.mimeType.indexOf("image/") >= 0
                            readonly property string filePath: "file://" + attachmentsFolder + "/" + modelData.fileName

                            RowLayout {
                                visible: !isImage && !modelData.text
                                Kirigami.Icon {
                                    scale: pointSize / Kirigami.Theme.defaultFont.pointSize
                                    source: modelData.iconName
                                }
                                Text {
                                    text: modelData.name
                                    color: content.textColor
                                    font.pointSize: pointSize
                                }
                                MouseArea {
                                    width: parent.width
                                    height: parent.height
                                    onDoubleClicked: Qt.openUrlExternally(filePath)
                                }
                            }

                            Image {
                                id: image
                                source: isImage ? filePath : ""
                                fillMode: Image.PreserveAspectFit
                                sourceSize.width: Math.round(delegateParent.width * 0.7)
                                height: Math.min(msgPage.height * 0.5, image.implicitHeight)
                                asynchronous: false
                                cache: false
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: if (!listView.moving) {
                                            pageStack.layers.push("qrc:/PreviewPage.qml", {
                                            filePath: filePath,
                                            type: modelData.mimeType
                                        } )
                                    }
                                }

                                Component.onCompleted: {
                                    if (content.attachments.length === 1 && isImage) {
                                        rect.color = "transparent"
                                        rect.border.color = "transparent"
                                        rect.padding = 0
                                    }
                                }

                                // rounded corners on image
                                layer.enabled: true
                                layer.effect: OpacityMask {
                                    maskSource: Item {
                                        width: image.width
                                        height: image.height
                                        Rectangle {
                                            anchors.fill: parent
                                            radius: Kirigami.Units.gridUnit / 2
                                        }
                                    }
                                }

                                AnimatedImage {
                                    source: parent.source && modelData.mimeType === "image/gif" ? parent.source : ""
                                    anchors.fill: parent
                                    cache: false
                                }
                            }

                            // text contents
                            Controls.Label {
                                visible: !!modelData.text
                                width: Math.min(delegateParent.width * 0.7, implicitWidth)
                                text: Utils.textToHtml(modelData.text)
                                maximumLineCount: 12
                                wrapMode: Text.Wrap
                                textFormat: Text.StyledText
                                linkColor: modelData.sentByMe ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.linkColor
                                color: content.textColor
                                font.pointSize: pointSize
                                font.family: "Noto Sans, Noto Color Emoji"
                                Component.onCompleted: content.clipped = truncated
                                onLinkActivated: Qt.openUrlExternally(link)
                            }
                        }
                    }

                    ColumnLayout {
                        visible: modelText.truncated || content.clipped
                        Rectangle {
                            width: content.implicitWidth
                            height: 1
                            color: Kirigami.Theme.disabledTextColor
                        }

                        RowLayout {
                            width: content.width

                            Kirigami.Icon {
                                Layout.maximumWidth: pointSize * 2
                                Layout.maximumHeight: pointSize * 2
                                source: "view-fullscreen"
                                color: content.textColor
                            }

                            Text {
                                text: i18n("View all")
                                color: content.textColor
                                font.pointSize: pointSize
                                font.capitalization: Font.AllUppercase
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Kirigami.Icon {
                                Layout.maximumWidth: pointSize * 2
                                Layout.maximumHeight: pointSize * 2
                                source: "arrow-right"
                                color: content.textColor
                            }
                        }
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

                Rectangle {
                    visible: rect.tapbacks
                    anchors.left: model.sentByMe ? undefined : parent.left
                    anchors.right: model.sentByMe ? parent.right : undefined
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: model.sentByMe ? undefined : parent.width - Kirigami.Units.largeSpacing * 2
                    anchors.rightMargin: model.sentByMe ? parent.width - Kirigami.Units.largeSpacing * 2 : undefined
                    anchors.bottomMargin: parent.height - Kirigami.Units.largeSpacing * 1.5
                    color: model.sentByMe ?  listView.incomingColor : listView.outgoingColor
                    height: tapback.height
                    width: tapback.width
                    radius: height / 2
                    border.width: 2
                    border.color: Kirigami.Theme.backgroundColor

                    Rectangle {
                        anchors.left: model.sentByMe ? undefined : parent.left
                        anchors.right: model.sentByMe ? parent.right : undefined
                        anchors.top: parent.top
                        anchors.leftMargin: model.sentByMe ? undefined : parent.width - height
                        anchors.rightMargin: model.sentByMe ? parent.width - height : undefined
                        anchors.topMargin: parent.height - height * 1.2
                        color: parent.color
                        width: Kirigami.Units.smallSpacing * 3
                        height: Kirigami.Units.smallSpacing * 3
                        radius: height / 2

                        Rectangle {
                            anchors.left: model.sentByMe ? undefined : parent.right
                            anchors.right: model.sentByMe ? parent.left : undefined
                            anchors.top: parent.top
                            anchors.leftMargin: model.sentByMe ? undefined : parent.width - height * 2
                            anchors.rightMargin: model.sentByMe ? parent.width - height * 2 : undefined
                            anchors.topMargin: parent.height - height / 2
                            color: parent.color
                            width: Kirigami.Units.smallSpacing * 1.5
                            height: Kirigami.Units.smallSpacing * 1.5
                            radius: height / 2
                        }
                    }

                    Row {
                        id: tapback
                        property color textColor: model.sentByMe ?  listView.incomingTextColor : listView.outgoingTextColor
                        padding: Kirigami.Units.largeSpacing

                        Repeater {
                            model: tapbackKeys.filter(key => rect.tapbacks && rect.tapbacks[key] && rect.tapbacks[key].length > 0)

                            Text {
                                text: modelData
                                font.family: "Noto Sans, Noto Color Emoji"
                                fontSizeMode: Text.Fit
                                minimumPixelSize: 10
                                font.pixelSize: 72
                                height: pointSize * 2
                                width: height
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: rect
                propagateComposedEvents: true
                onPressAndHold: {
                    menu.index = index
                    menu.id = model.id
                    menu.text = model.text
                    menu.attachments = content.attachments
                    menu.smil = model.smil
                    menu.resend = model.sentByMe && model.deliveryState == MessageModel.Failed
                    menu.tapbacks = rect.tapbacks
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
        property var attachments: []
        property string smil
        property bool resend: false
        property var tapbacks
        property string sendingNumber: Utils.phoneNumberToInternationalString(Utils.phoneNumber(Utils.sendingNumber()))

        edge: Qt.BottomEdge

        contentItem: ColumnLayout {
            RowLayout {
                Repeater {
                    model: tapbackKeys
                    ColumnLayout {
                        property int count: menu.tapbacks &&  menu.tapbacks[modelData] ? menu.tapbacks[modelData].length : 0

                        Controls.RoundButton {
                            padding: Kirigami.Units.largeSpacing * 2
                            flat: !highlighted
                            highlighted: count > 0 && menu.tapbacks[modelData].indexOf(menu.sendingNumber) >= 0
                            onPressed: {
                                if (!menu.tapbacks) {
                                    menu.tapbacks = {}
                                }

                                if (!menu.tapbacks[modelData]) {
                                    menu.tapbacks[modelData] = []
                                }

                                const isRemoved = menu.tapbacks[modelData].indexOf(menu.sendingNumber) >= 0
                                messageModel.sendTapback(menu.id, modelData, isRemoved)
                                menu.close()
                            }

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                font.family: "Noto Sans, Noto Color Emoji"
                                fontSizeMode: Text.Fit
                                minimumPixelSize: 10
                                font.pixelSize: 72
                                height: pointSize * 3
                                width: height
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: count > 0 ? count : ""
                            color: Kirigami.Theme.disabledTextColor
                            font.pointSize: pointSize - 2
                        }
                    }
                }
            }
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
                visible: menu.text || menu.attachments.reduce((a,c) => a += (c.text || ""), "")
                text: i18n("Copy text")
                icon: "edit-copy"
                onClicked: {
                    Utils.copyTextToClipboard(menu.text || menu.attachments.reduce((a,c) => a += (c.text || ""), ""))
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                visible: menu.attachments.length > 0
                text: i18n("Save attachment")
                icon: "mail-attachment-symbolic"
                onClicked: {
                    attachmentList.selected = []
                    attachmentList.items = menu.attachments.filter(o => o.fileName)
                    attachmentList.open()
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                visible: menu.smil
                text: i18n("View slideshow")
                icon: "view-presentation-symbolic"
                onClicked: {
                    pageStack.layers.push("qrc:/SlideshowPage.qml", {
                        recipients: msgPage.title,
                        attachments: menu.attachments,
                        folder: attachmentsFolder,
                        smil: menu.smil
                    } )
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                text: i18n("Delete message")
                icon: "edit-delete"
                onClicked: {
                    listView.currentIndex = menu.index
                    messageModel.deleteMessage(menu.id, menu.index, menu.attachments.map(o => o.fileName))
                    menu.close()
                }
            }
            Kirigami.BasicListItem {
                visible: menu.resend
                text: i18nc("Retry sending message", "Resend")
                icon: "edit-redo"
                onClicked: {
                    messageModel.sendMessage(menu.text, menu.attachments.map(o => "file://" + attachmentsFolder + "/" + o.fileName), menu.attachments.reduce((a,c) => a += (c.size || 0), 0))
                    listView.currentIndex = menu.index
                    messageModel.deleteMessage(menu.id, menu.index, menu.attachments.map(o => o.fileName))
                    menu.close()
                    delayCountChanged.restart()
                }
            }
        }
    }

    Kirigami.OverlaySheet {
        id: attachmentList

        property var items: []
        property var selected: []

        title: i18n("Save attachment")

        ListView {
            model: attachmentList.items
            implicitWidth: Kirigami.Units.gridUnit * 30
            onCountChanged: currentIndex = -1

            delegate: Kirigami.AbstractListItem {
                Controls.CheckBox {
                    Layout.fillWidth: true
                    checked: attachmentList.selected.indexOf(modelData.fileName) >= 0
                    text: (modelData.name || modelData.fileName)
                    onClicked: {
                        const index = attachmentList.selected.indexOf(modelData.fileName)
                        if (index >=0) {
                            attachmentList.selected.splice(index, 1)
                        } else {
                            attachmentList.selected.push(modelData.fileName)
                        }
                        attachmentList.selected = attachmentList.selected
                    }
                }
            }
        }

        footer: Row {
            spacing: Kirigami.Units.gridUnit
            layoutDirection: Qt.RightToLeft

            Controls.Button {
                enabled: attachmentList.selected.length > 0
                text: i18n("Save")
                onClicked: {
                    messageModel.saveAttachments(attachmentList.selected)
                    attachmentList.close()
                }
            }
            Controls.Button {
                text: i18n("Cancel")
                onClicked: attachmentList.close()
            }
        }
    }

    footer: ColumnLayout {
        spacing: 0
        width: parent.width

        Controls.ScrollView {
            id: scrollView
            Layout.fillWidth: true
            bottomPadding: 0
            implicitWidth: flow.implicitWidth
            implicitHeight: files.count > 0 ? Math.min(msgPage.availableHeight - composeArea.height - Kirigami.Units.largeSpacing, flow.implicitHeight) : 1
            contentWidth: availableWidth
            contentHeight: flow.implicitHeight
            clip: true
            background: Rectangle {
                color: Kirigami.Theme.alternateBackgroundColor
            }

            Flow {
                id: flow
                anchors.fill: parent
                padding: Kirigami.Units.smallSpacing
                spacing: 0

                Repeater {
                    model: files
                    Item {
                        width: fileItem.width + Kirigami.Units.largeSpacing
                        height: fileItem.height + Kirigami.Units.largeSpacing

                        property bool isImage: mimeType.indexOf("image/") >= 0

                        Rectangle {
                            id: fileItem
                            anchors.centerIn: parent
                            implicitWidth: (isImage ? attachImg.width : layout.implicitWidth) + 2
                            implicitHeight: (isImage ? attachImg.implicitHeight : layout.implicitHeight) + 2
                            border.width: 1
                            border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                            color: Kirigami.Theme.backgroundColor
                            radius: Kirigami.Units.largeSpacing

                            RowLayout {
                                id: layout
                                visible: !isImage
                                Kirigami.Icon {
                                    source: iconName
                                }
                                Text {
                                    text: name
                                    color: Kirigami.Theme.textColor
                                }
                                Item {
                                    width: Kirigami.Units.largeSpacing
                                }
                                MouseArea {
                                    width: parent.width
                                    height: parent.height
                                    onDoubleClicked: Qt.openUrlExternally(filePath)
                                }
                            }

                            Image {
                                id: attachImg
                                anchors.centerIn: parent
                                source: isImage ? filePath : ""
                                sourceSize.height: Kirigami.Units.gridUnit * 8
                                width: Math.min(flow.width - Kirigami.Units.largeSpacing * 2, implicitWidth)
                                cache: false
                                fillMode: Image.PreserveAspectCrop

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: pageStack.layers.push("qrc:/PreviewPage.qml", {
                                        filePath: filePath,
                                        type: mimeType
                                    } )
                                }

                                // rounded corners on image
                                layer.enabled: true
                                layer.effect: OpacityMask {
                                    maskSource: Item {
                                        width: attachImg.width
                                        height: attachImg.height
                                        Rectangle {
                                            anchors.fill: parent
                                            radius: Kirigami.Units.largeSpacing
                                        }
                                    }
                                }

                                AnimatedImage {
                                    source: parent.source && mimeType === "image/gif" ? parent.source : ""
                                    anchors.fill: parent
                                }
                            }
                        }

                        Controls.RoundButton {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            icon.name: "remove"
                            icon.color: Kirigami.Theme.negativeTextColor
                            icon.width: Kirigami.Units.gridUnit * 1.5
                            icon.height: Kirigami.Units.gridUnit * 1.5
                            padding: 0
                            width: Kirigami.Units.gridUnit * 1.5
                            height: Kirigami.Units.gridUnit * 1.5
                            onPressed: files.remove(index)
                            Controls.ToolTip.delay: 1000
                            Controls.ToolTip.timeout: 5000
                            Controls.ToolTip.visible: hovered
                            Controls.ToolTip.text: i18nc("Remove item from list", "Remove")
                        }
                    }
                }
            }

            Text {
                visible: files.count > 0
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                bottomPadding: textarea.lineCount < 3 && textarea.length > 0 ? Kirigami.Units.gridUnit : 0
                text: files.count > 0 ? formatBytes(filesTotalSize()) : ""
                color: Kirigami.Theme.disabledTextColor
            }
        }

        RowLayout {
            id: composeArea

            Controls.Button {
                id: attachAction
                Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                Layout.margins: Kirigami.Units.smallSpacing
                icon.name: "mail-attachment-symbolic"
                icon.width: Kirigami.Units.iconSizes.smallMedium
                icon.height: Kirigami.Units.iconSizes.smallMedium
                flat: true
                hoverEnabled: false
                onPressed: fileDialog.open()
            }

            Controls.TextArea {
                id: textarea
                Layout.fillWidth: true
                Layout.minimumHeight: sendButton.height + Kirigami.Units.smallSpacing * 2
                verticalAlignment: TextEdit.AlignVCenter
                placeholderText: {
                    var number = Utils.sendingNumber()
                    if (number === "0") {
                        return i18n("Write Message...")
                    } else {
                        return i18nc("%1 is a phone number", "Send Message from %1...", number)
                    }
                }
                font.pointSize: pointSize
                font.family: "Noto Sans, Noto Color Emoji"
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                background: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                }
                inputMethodHints: Qt.ImhNoPredictiveText
            }

            Controls.Action {
                id: sendAction
                onTriggered: {
                    msgPage.forceActiveFocus()
                    messageModel.sendMessage(textarea.text, filesToList(), filesTotalSize())
                    files.clear()
                    textarea.text = ""
                }
            }

            Shortcut{
                sequences: ["Ctrl+Enter", "Ctrl+Return"]
                onActivated: sendAction.trigger()
            }

            Controls.Button {
                id: sendButton
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                Layout.margins: Kirigami.Units.smallSpacing
                icon.name: "document-send"
                icon.width: Kirigami.Units.iconSizes.smallMedium
                icon.height: Kirigami.Units.iconSizes.smallMedium
                hoverEnabled: false
                enabled: (textarea.length > 0 || files.count > 0) && !maxAttachmentsError.visible
                onPressed: sendAction.trigger()

                Controls.Label {
                    text: textarea.length
                    font: Kirigami.Theme.smallFont
                    color: Kirigami.Theme.disabledTextColor
                    visible: textarea.length > 0
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    anchors.margins: Kirigami.Units.smallSpacing * 1.5
                }
            }
        }

        Kirigami.InlineMessage {
            id: duplicateNotify
            width: parent.width
            type: Kirigami.MessageType.Warning
            text: i18n("Duplicate recipient")
            visible: false
        }

        FileDialog {
            id: fileDialog
            title: i18n("Choose a file")
            folder: shortcuts.pictures
            selectMultiple: true
            onAccepted: {
                for (let i = 0; i < fileDialog.fileUrls.length; i++) {
                    files.append(messageModel.fileInfo(fileDialog.fileUrls[i]))
                }
            }
        }
    }
}
