// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.spacebar

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

            Delegates.RoundedItemDelegate {
                id: delegateItem
                width: parent.width
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

                    Controls.Label {
                        Layout.fillWidth: true
                        text: i18n("Add people")
                        color: Kirigami.Theme.textColor
                    }
                }
                onClicked: pageStack.push("qrc:/NewConversationPage.qml", { selected: people.slice() })
            }
        }

        clip: true
        reuseItems: false
        currentIndex: -1

        model: people

        delegate: Delegates.RoundedItemDelegate {
            id: delegateItem
            width: contactsList.width
            implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
            verticalPadding: 0
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Components.Avatar {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    source: modelData.phoneNumber ? "image://avatar/" + modelData.phoneNumber : ""
                    name: modelData.name
                    imageMode: Components.Avatar.ImageMode.AdaptiveImageOrInitals
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Controls.Label {
                        id: labelItem
                        Layout.fillWidth: true
                        Layout.alignment: subtitleItem.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
                        text: modelData.name || Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.phoneNumber))
                        elide: Text.ElideRight
                        color: Kirigami.Theme.textColor
                    }
                    Controls.Label {
                        id: subtitleItem
                        visible: text
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        text: modelData.name ? Utils.phoneNumberToInternationalString(Utils.phoneNumber(modelData.phoneNumber)) : ""
                        elide: Text.ElideRight
                        color: Kirigami.Theme.textColor
                        opacity: 0.7
                        font: Kirigami.Theme.smallFont
                    }
                }
            }
            onPressAndHold: {
                showPassiveNotification("Number copied to clipboard", 3000);
                Utils.copyTextToClipboard(modelData.phoneNumber)
            }
            onClicked: Utils.launchPhonebook()
        }
    }
}
