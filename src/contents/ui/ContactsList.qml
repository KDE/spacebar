// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.15 as Kirigami
import org.kde.people 1.0 as KPeople

import org.kde.spacebar 1.0

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
            const pop = numberPopup.createObject(parent, {numbers: phoneNumbers, title: i18n("Select a number")})
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
            const index = contactsProxyModel.match(contactsProxyModel.index(0,0), 0, az.itemAt(i).text, 1, Qt.MatchStartsWith)[0]

            if (index) {
                contactsList.positionViewAtIndex(index.row, ListView.Beginning)
                break
            }
        }
        if (i === az.count) {
            contactsList.positionViewAtEnd()
        }
    }

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
                    text: selected.length + " " + i18n("Selected")
                    color: Kirigami.Theme.disabledTextColor
                }

                Row {
                    Layout.fillWidth: true
                    layoutDirection: Qt.RightToLeft
                    padding: Kirigami.Units.smallSpacing
                    Controls.Button {
                        id: compose
                        text: i18n("Compose")
                        onClicked: {
                            let numbers = selected.map(o => o.phoneNumber)
                            if (numbers.length === 0) {
                                numbers.push(searchField.text)
                            }
                            contactsList.clicked(numbers)
                        }
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

            Kirigami.BasicListItem {
                Layout.fillWidth: true
                visible: contactsList.count === 0
                backgroundColor: showSections ? "transparent" : Kirigami.Theme.backgroundColor
                icon: "list-add"
                iconSize: Kirigami.Units.iconSizes.medium
                iconColor: Kirigami.Theme.textColor
                text: searchField.text
                subtitle: isNaN(searchField.text) ? alphaToNumeric(searchField.text) : ""
                separatorVisible: false
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
        text: section.toUpperCase()
    }
    clip: true
    reuseItems: true
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

    boundsBehavior: Flickable.StopAtBounds

    delegate: Kirigami.BasicListItem {
        id: contact
        width: contactsList.width
        height: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing
        visible: showAll || searchText.length > 0
        backgroundColor: showSections ? "transparent" : Kirigami.Theme.backgroundColor
        highlighted: false
        separatorVisible: !showSections
        label: model.display
        subtitle: showNumber ? Utils.phoneNumberToInternationalString(Utils.phoneNumber(model.phoneNumber)) : ""
        leading: RowLayout {
            Controls.CheckBox {
                visible: multiSelect
                Layout.fillHeight: true
                checked: isSelected(model.personUri)
                checkable: false
                onToggled: selectNumber(model.personUri, model.name)
            }

            Kirigami.Avatar {
                Layout.fillHeight: true
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                source: model.phoneNumber ? "image://avatar/" + model.phoneNumber : ""
                name: model.display
                imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
            }
        }
        onClicked: selectNumber(model.personUri, model.name)
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
        width: Kirigami.Units.gridUnit
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: Kirigami.Theme.disabledTextColor
        radius: width / 2

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            Repeater {
                id: az
                model:parent.height < 320 ? [
                "A","C","E","G","I","K","M","O","Q","S","U","W","Z"] : [
                "A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"]
                Text {
                    text: modelData
                    font: Kirigami.Theme.smallFont
                    color: Kirigami.Theme.disabledTextColor
                    Layout.alignment: Qt.AlignHCenter

                    MouseArea {
                        z: 3
                        anchors.fill: parent
                        propagateComposedEvents: false
                        onClicked: quickScroll(index)
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        z: -1
        propagateComposedEvents: true
    }
}
