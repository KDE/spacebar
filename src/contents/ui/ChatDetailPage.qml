// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.15 as Kirigami

import org.kde.spacebar 1.0

Kirigami.ScrollablePage {
    title: i18n("Details")
    padding: 0

    property var people

    ListView {
        id: contactsList

        header: Column {
            width: parent.width

            Kirigami.Heading {
                level: 5
                type: Kirigami.Heading.Type.Primary
                Layout.fillWidth: true
                leftPadding: Kirigami.Units.largeSpacing * 2
                text: i18np("%1 person", "%1 people", people.length)
                color: Kirigami.Theme.disabledTextColor
            }

            Kirigami.BasicListItem {
                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
                leading: Rectangle {
                    width: height
                    radius: height / 2
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
                text: i18n("Add people")
                separatorVisible: false
                onClicked: pageStack.push("qrc:/NewConversationPage.qml", { selected: people.slice() })
            }
        }

        clip: true
        reuseItems: false
        currentIndex: -1

        model: people

        delegate: Kirigami.BasicListItem {
            width: contactsList.width - Kirigami.Units.smallSpacing
            implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
            highlighted: false
            separatorVisible: false
            label: modelData.name || Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.phoneNumber))
            subtitle: modelData.name ? Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.phoneNumber)) : ""
            leading: Kirigami.Avatar {
                width: height
                source: modelData.phoneNumber ? "image://avatar/" + modelData.phoneNumber : ""
                name: modelData.name
                imageMode: Kirigami.Avatar.AdaptiveImageOrInitals
            }
            onPressAndHold: {
                showPassiveNotification("Number copied to clipboard", 3000);
                Utils.copyTextToClipboard(modelData.phoneNumber)
            }
            onClicked: Utils.launchPhonebook()
        }
    }
}
