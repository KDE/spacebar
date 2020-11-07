// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

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

    Q_SIGNAL void messagesChanged(const QString &phoneNumber);

    // Fetch requessts
    Q_SIGNAL void requestAddMessage(const Message &message);
    Q_SIGNAL void requestMessagesForNumber(const QString &phoneNumber);
    Q_SIGNAL void requestLastId();
    Q_SIGNAL void requestMarkMessageDelivered(const int id);
    Q_SIGNAL void requestMarkMessageRead(const int id);
    Q_SIGNAL void requestChats();
    Q_SIGNAL void requestUnreadMessagesForNumber(const QString &phoneNumber);
    Q_SIGNAL void requestLastMessageForNumber(const QString &phoneNumber);
    Q_SIGNAL void requestLastContactedForNumber(const QString &phoneNumber);
    Q_SIGNAL void requestMarkChatAsRead(const QString &phoneNumber);

    // Responses
    Q_SIGNAL void messagesFetchedForNumber(const QString &phoneNumber, const QVector<Message> messages);
    Q_SIGNAL void lastIdFetched(const int id);
    Q_SIGNAL void chatsFetched(QVector<Chat> chats);
    Q_SIGNAL void unreadMessagesFetchedForNumber(const QString &phoneNumber, const int unreadMessages);
    Q_SIGNAL void lastMessageFetchedForNumber(const QString &phoneNumber, const QString &message);
    Q_SIGNAL void lastContactedFetchedForNumber(const QString &phoneNumber, const QDateTime &lastContacted);

private:
    Q_SLOT void addMessage(const Message &message);
    Q_SLOT void messagesForNumber(const QString &phoneNumber);
    Q_SLOT void lastId();
    Q_SLOT void markMessageDelivered(const int id);
    Q_SLOT void markMessageRead(const int id);
    Q_SLOT void chats();
    Q_SLOT void unreadMessagesForNumber(const QString &phoneNumber);
    Q_SLOT void lastMessageForNumber(const QString &phoneNumber);
    Q_SLOT void lastContactedForNumber(const QString &phoneNumber);
    Q_SLOT void markChatAsRead(const QString &phoneNumber);

    Database m_database;
};
