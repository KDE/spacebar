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
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1
import org.kde.telepathy 0.1
import org.kde.kirigami 2.4 as Kirigami

Controls.TextArea {
    id: messageTextField

    wrapMode: TextEdit.WordWrap
    textFormat: TextEdit.RichText

    background: Item {}

    property int acc: -1
    //property alias emojisAutocompletionModel: emojisAutocompletionFilter.sourceModel
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

    Keys.onReturnPressed: {
        if (event.modifiers === Qt.NoModifier) {
            if (conversation.canSendMessages) {
                view.model.sendNewMessage(emojisHandler.getText());
                text = "";
            } else {
                // TODO better text
                showPassiveNotification(i18n("You need to connect first"), 3000);
            }
        } else if (event.modifiers !== Qt.NoModifier) {
            event.accepted = false;
        }
    }
}
