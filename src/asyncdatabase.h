// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include "database.h"

///
/// \brief The AsyncDatabase class provides an asynchronous API around the Database class
///
/// It can be used for running the Database on a separate thread.
///
class AsyncDatabase : public QObject
{
    Q_OBJECT

public:
    explicit AsyncDatabase();

    // This class shall never expose anything but signals

    Q_SIGNAL void messagesChanged(const PhoneNumberList &phoneNumberList);

    // Fetch requessts
    Q_SIGNAL void requestAddMessage(const Message &message);
    Q_SIGNAL void requestDeleteMessage(const QString &id);
    Q_SIGNAL void requestMessagesForNumber(const PhoneNumberList &phoneNumberList);
    Q_SIGNAL void requestUpdateMessageDeliveryState(const QString &id, const MessageState state);
    Q_SIGNAL void requestMarkMessageRead(const int id);
    Q_SIGNAL void requestChats();
    Q_SIGNAL void requestUnreadMessagesForNumber(const PhoneNumberList &phoneNumberList);
    Q_SIGNAL void requestLastMessageForNumber(const PhoneNumberList &phoneNumberList);
    Q_SIGNAL void requestLastContactedForNumber(const PhoneNumberList &phoneNumberList);
    Q_SIGNAL void requestMarkChatAsRead(const PhoneNumberList &phoneNumberList);
    Q_SIGNAL void requestDeleteChat(const PhoneNumberList &phoneNumberList);

    // Responses
    Q_SIGNAL void messagesFetchedForNumber(const PhoneNumberList &phoneNumberList, const QVector<Message> messages);
    Q_SIGNAL void chatsFetched(QVector<Chat> chats);
    Q_SIGNAL void unreadMessagesFetchedForNumber(const PhoneNumberList &phoneNumberList, const int unreadMessages);
    Q_SIGNAL void lastMessageFetchedForNumber(const PhoneNumberList &phoneNumberList, const QString &message);
    Q_SIGNAL void lastContactedFetchedForNumber(const PhoneNumberList &phoneNumberList, const QDateTime &lastContacted);

private:
    Q_SLOT void addMessage(const Message &message);
    Q_SLOT void deleteMessage(const QString &id);
    Q_SLOT void messagesForNumber(const PhoneNumberList &phoneNumberList);
    Q_SLOT void updateMessageDeliveryState(const QString &id, const MessageState state);
    Q_SLOT void markMessageRead(const int id);
    Q_SLOT void chats();
    Q_SLOT void unreadMessagesForNumber(const PhoneNumberList &phoneNumberList);
    Q_SLOT void lastMessageForNumber(const PhoneNumberList &phoneNumberList);
    Q_SLOT void lastContactedForNumber(const PhoneNumberList &phoneNumberList);
    Q_SLOT void markChatAsRead(const PhoneNumberList &phoneNumberList);
    Q_SLOT void deleteChat(const PhoneNumberList &phoneNumberList);

    Database m_database;
};
