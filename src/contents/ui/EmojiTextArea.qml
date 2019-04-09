/**
 *   Copyright 2016 Martin Klapetek <mklapetek@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import org.kde.telepathy 0.1
import org.kde.kirigami 2.1 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore

TextArea {
    id: messageTextField

    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    wrapMode: TextEdit.WordWrap
    textFormat: TextEdit.RichText

    property int acc: -1
    property alias emojisAutocompletionModel: emojisAutocompletionFilter.sourceModel
    property alias lineSpacing: fontMetrics.lineSpacing

    function getEmojiText() {
        return emojisHandler.getText();
    }

    FontMetrics {
        id: fontMetrics
        font: messageTextField.font
    }

    TextAreaEmojisHandler {
        id: emojisHandler
        textArea: messageTextField
    }

    Rectangle {
        id: autoSuggest
        color: "white"
        width: parent.width
        height: Math.min(emojiAutocompletionView.count * 32, 32 * 9)
        visible: false
        y: -height - units.smallSpacing
        clip: true

        function hide() {
            emojisAutocompletionFilter.filterString = "";
            messageTextField.acc = -1;
            autoSuggest.visible = false;
        }

        ListView {
            id: emojiAutocompletionView
            anchors.fill: parent
            model: PlasmaCore.SortFilterModel {
                id: emojisAutocompletionFilter
                filterRole: "emojiText"
                sortRole: "emojiText"
            }

            delegate: MouseArea {
                width: parent.width - units.smallSpacing
                height: 32
                x: units.smallSpacing
                z: 250

                onClicked: {
                    messageTextField.selectWord();
                    messageTextField.remove(messageTextField.selectionStart, messageTextField.selectionEnd);
                    messageTextField.insert(messageTextField.cursorPosition, model.emojiText.substr(1) + " ");
                    autoSuggest.visible = false;
                    messageTextField.acc = -1;
                    emojisAutocompletionFilter.filterString = "";
                }

                PlasmaCore.IconItem {
                    id: emojiIcon
                    height: 24
                    width: 24
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    source: model.emojiFullPath
                }

                Kirigami.Label {
                    anchors.left: emojiIcon.right
                    anchors.right: parent.right
                    anchors.verticalCenter: emojiIcon.verticalCenter
                    anchors.margins: units.largeSpacing
                    textFormat: Text.StyledText
                    text: model.emojiText ? model.emojiText.replace(emojisAutocompletionFilter.filterString,
                                                  "<b>" + emojisAutocompletionFilter.filterString + "</b>")
                                          : "";
                }
            }
        }
    }

    Keys.onPressed: {
        if (event.text == ":") {
            if (acc > 0) {
                autoSuggest.hide();
            } else {
                acc = 0;
            }
            return;
        }

        if (acc == -1) {
            return;
        }

        if (event.key == Qt.Key_Space || event.key == Qt.Key_Escape) {
            autoSuggest.hide();
        }

        acc++;

        if (event.key == Qt.Key_Backspace) {
            emojisAutocompletionFilter.filterString = emojisAutocompletionFilter.filterString.slice(0, -1);
            if (emojisAutocompletionFilter.filterString.length == 0) {
                autoSuggest.hide();
            }
        } else {
            emojisAutocompletionFilter.filterString = emojisAutocompletionFilter.filterString + event.text;
        }

        if (acc >= 2) {
            autoSuggest.visible = true;
        }
    }

    Keys.onReturnPressed: {
        if (event.modifiers == Qt.NoModifier) {
            if (conversation.canSendMessages) {
                view.model.sendNewMessage(emojisHandler.getText());
                text = "";
            } else {
                // TODO better text
                showPassiveNotification(i18n("You need to connect first"), 3000);
            }
        } else if (event.modifiers != Qt.NoModifier) {
            event.accepted = false;
        }
    }
}
