// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.people as KPeople

import org.kde.spacebar

ListView {
    id: contactsList

    property var selected: []
    property bool multiSelect: false
    property bool showAll: true
    property bool showNumber: false
    property bool showSections: false
    property string searchText

    signal clicked(var numbers)
    signal select(var number)

    Component {
        id: numberPopup

        PhoneNumberDialog {}
    }

    function formatNumber(number) {
        return Utils.phoneNumberToInternationalString(Utils.phoneNumber(number))
    }

    function selectNumber(personUri, name) {
        const phoneNumbers = Utils.phoneNumbers(personUri)

        if (phoneNumbers.length === 1) {
            modifySelection(formatNumber(phoneNumbers[0].normalizedNumber), name)
        } else {
            const pop = numberPopup.createObject(parent, {
                numbers: phoneNumbers,
                title: i18n("Select a number"),
                selected: selected
            })
            pop.onNumberSelected.connect(number => modifySelection(formatNumber(number), name))
            pop.open()
        }
    }

    function modifySelection(number, name) {
        contactsList.select(number)
        const index = selected.findIndex(o => o.phoneNumber == number)
        if (index == -1) {
            selected.push({name: name, phoneNumber: number})
        } else {
            selected.splice(index, 1)
        }
        selected = selected
    }

    function isSelected(personUri) {
        return Utils.phoneNumbers(personUri).find(number => {
            const normalized = formatNumber(number.normalizedNumber)
            return selected.findIndex(o => o.phoneNumber == normalized) != -1
        }) ? true : false
    }

    function alphaToNumeric(text) {
        const chars = text.split("")
        for (let i = 0; i < chars.length; i++) {
            chars[i] = chars[i].toUpperCase()
            switch (chars[i]) {
                case "A":
                case "B":
                case "C":
                    chars[i] = 2
                    break;
                case "D":
                case "E":
                case "F":
                    chars[i] = 3
                    break;
                case "G":
                case "H":
                case "I":
                    chars[i] = 4
                    break;
                case "J":
                case "K":
                case "L":
                    chars[i] = 5
                    break;
                case "M":
                case "N":
                case "O":
                    chars[i] = 6
                    break;
                case "P":
                case "Q":
                case "R":
                case "S":
                    chars[i] = 7
                    break;
                case "T":
                case "U":
                case "V":
                    chars[i] = 8
                    break;
                case "W":
                case "X":
                case "Y":
                case "Z":
                    chars[i] = 9
                    break;
                default:
                    chars[i] = 0
            }
        }

        return chars.join("")
    }

    function quickScroll(index) {
        let i
        for (i = index; i < az.count; i++) {
            const index = contactsProxyModel.match(contactsProxyModel.index(0,0), 0, az.itemAt(i).contentItem.text, 1, Qt.MatchStartsWith)[0]

            if (index) {
                contactsList.positionViewAtIndex(index.row, ListView.Beginning)
                break
            }
        }
        if (i === az.count) {
            contactsList.positionViewAtEnd()
        }
    }

    MouseArea {
        anchors.fill: contactsList.contentItem
        onPressed: mouse => mouse.accepted = false
    }

    pressDelay: Kirigami.Settings.isMobile ? 200 : 0

    headerPositioning: ListView.OverlayHeader
    header: Rectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        width: parent.width
        height: headerColumn.height
        z: 3
        color: Kirigami.Theme.backgroundColor

        ColumnLayout {
            id: headerColumn
            width: parent.width

            RowLayout {
                visible: multiSelect
                Layout.fillWidth: true
                height: compose.height

                Kirigami.Heading {
                    padding: Kirigami.Units.largeSpacing
                    level: 4
                    type: Kirigami.Heading.Type.Normal
                    text: i18nc("Number of items selected", "%1 Selected", selected.length)
                    color: Kirigami.Theme.disabledTextColor
                }

                Row {
                    Layout.fillWidth: true
                    layoutDirection: Qt.RightToLeft
                    padding: Kirigami.Units.smallSpacing
                    Controls.Button {
                        id: compose
                        text: i18nc("Open chat conversation window", "Next")
                        onClicked: contactsList.clicked(selected.map(o => o.phoneNumber))
                    }
                }
            }

            Controls.Control {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing
                topPadding: 0

                contentItem: Kirigami.ActionTextField {
                    background: Rectangle {
                        anchors.fill: parent
                        color: Kirigami.Theme.alternateBackgroundColor
                    }

                    id: searchField
                    onTextChanged: {
                        contactsProxyModel.setFilterFixedString(text)
                        searchText = text
                    }
                    inputMethodHints: Qt.ImhNoPredictiveText
                    placeholderText: i18n("Search or enter number…")
                    focusSequence: "Ctrl+F"
                    rightActions: [
                        Kirigami.Action {
                            icon.name: "edit-delete-remove"
                            visible: searchField.text.length > 0
                            onTriggered: {
                                searchField.text = ""
                                searchField.accepted()
                            }
                        }
                    ]
                }
            }

            Delegates.RoundedItemDelegate {
                id: delegateItem
                visible: searchField.text.length > 0
                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
                verticalPadding: 0
                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Rectangle {
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        radius: width / 2
                        border.color: Kirigami.Theme.linkColor
                        border.width: 2
                        color: "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: "+"
                            color: Kirigami.Theme.linkColor
                            font.bold: true
                            font.pixelSize: parent.height / 1.5
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Controls.Label {
                            id: labelItem
                            Layout.fillWidth: true
                            Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
                            text: searchField.text
                            elide: Text.ElideRight
                            color: Kirigami.Theme.textColor
                        }
                        Controls.Label {
                            id: subtitleItem
                            visible: text
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            text: isNaN(searchField.text) ? alphaToNumeric(searchField.text) : ""
                            elide: Text.ElideRight
                            color: Kirigami.Theme.textColor
                            opacity: 0.7
                            font: Kirigami.Theme.smallFont
                        }
                    }

                }
                onClicked: {
                    const text = isNaN(searchField.text) ? alphaToNumeric(searchField.text) : searchField.text
                    modifySelection(text, text)
                    searchField.text = ""
                }
            }
        }
    }

    section.property: showSections && searchText === "" ? "display" : ""
    section.criteria: ViewSection.FirstCharacter
    section.delegate: Kirigami.ListSectionHeader {
        width: contactsList.width - Kirigami.Units.smallSpacing
        text: section.toUpperCase()
    }
    clip: true
    reuseItems: false
    currentIndex: -1

    model: KPeople.PersonsSortFilterProxyModel {
        id: contactsProxyModel
        sourceModel: KPeople.PersonsModel {
            id: contactsModel
        }
        requiredProperties: "phoneNumber"
        filterRole: Qt.DisplayRole
        sortRole: Qt.DisplayRole
        filterCaseSensitivity: Qt.CaseInsensitive
        Component.onCompleted: sort(0)
    }

    interactive: showAll || searchText.length > 0

    delegate: Delegates.RoundedItemDelegate {
        property bool selected: isSelected(model.personUri)

        id: delegateItem
        visible: showAll || searchText.length > 0
        width: contactsList.width
        implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
        verticalPadding: 0
        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Components.Avatar {
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                source: model.phoneNumber ? "image://avatar/" + model.phoneNumber : ""
                name: model.display
                imageMode: Components.Avatar.ImageMode.AdaptiveImageOrInitals

                Rectangle {
                    anchors.fill: parent
                    radius: width * 0.5
                    color: Kirigami.Theme.highlightColor
                    visible: selected

                    Kirigami.Icon {
                        anchors.fill: parent
                        source: "checkbox"
                        color: Kirigami.Theme.highlightedTextColor
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Controls.Label {
                    id: labelItem
                    Layout.fillWidth: true
                    Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
                    text: model.display
                    elide: Text.ElideRight
                    color: Kirigami.Theme.textColor
                }
                Controls.Label {
                    id: subtitleItem
                    visible: text
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    text: showNumber ? Utils.phoneNumberToInternationalString(Utils.phoneNumber(model.phoneNumber)) : ""
                    elide: Text.ElideRight
                    color: Kirigami.Theme.textColor
                    opacity: 0.7
                    font: Kirigami.Theme.smallFont
                }
            }

        }
        onReleased: selectNumber(model.personUri, model.name)
    }

    Kirigami.PlaceholderMessage {
        id: noContacts
        anchors.centerIn: parent
        text: i18n("No contacts with phone numbers yet")
        visible: contactsProxyModel.rowCount() === 0
        helpfulAction: Kirigami.Action {
            text: i18n("Open contacts app")
            onTriggered: Utils.launchPhonebook()
        }
    }

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: i18n("No results found")
        visible: contactsList.count === 0 && !noContacts.visible
    }

    Rectangle {
        visible: showAll && searchText === "" && contactsList.count > 0 && !noContacts.visible
        anchors.right: parent.right
        anchors.rightMargin: Kirigami.Units.smallSpacing
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: contactsList.headerItem.height / 2
        height: contactsList.height - contactsList.headerItem.height - Kirigami.Units.largeSpacing
        width: Kirigami.Units.gridUnit * 1.5
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Kirigami.Theme.disabledTextColor
        radius: width / 2

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            Repeater {
                id: az
                model: parent.height < 320 ? [
                "A","C","E","G","I","K","M","O","Q","S","U","W","Z"] : [
                "A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"]

                Controls.Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    flat: true
                    contentItem: Text {
                        text: modelData
                        font: Kirigami.Theme.smallFont
                        color: Kirigami.Theme.disabledTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onPressed: quickScroll(index)
                }
            }
        }
    }
}
