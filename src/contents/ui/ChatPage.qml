import QtQuick 2.0

import org.kde.kirigami 2.2 as Kirigami

import org.kde.spacebear 1.0

Kirigami.ScrollablePage {
    property MessageModel messageModel;

    ListView {
        model: messageModel
        delegate: Kirigami.BasicListItem {
            text: model.text
        }
    }
}
