// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "asyncdatabase.h"


AsyncDatabase::AsyncDatabase()
{
    qRegisterMetaType<Message>();
    qRegisterMetaType<Chat>();
    qRegisterMetaType<QVector<Chat>>();
    qRegisterMetaType<QVector<Message>>();

    // Forward messagesChanged signal
    connect(&m_database, &Database::messagesChanged, this, &AsyncDatabase::messagesChanged);

    // Connect requests to slots
    connect(this, &AsyncDatabase::requestAddMessage, this, &AsyncDatabase::addMessage);
    connect(this, &AsyncDatabase::requestLastId, this, &AsyncDatabase::lastId);
    connect(this, &AsyncDatabase::requestMarkMessageDelivered, this, &AsyncDatabase::markMessageDelivered);
    connect(this, &AsyncDatabase::requestMarkMessageRead, this, &AsyncDatabase::markMessageRead);
    connect(this, &AsyncDatabase::requestMessagesForNumber, this, &AsyncDatabase::messagesForNumber);
    connect(this, &AsyncDatabase::requestChats, this, &AsyncDatabase::chats);
    connect(this, &AsyncDatabase::requestUnreadMessagesForNumber, this, &AsyncDatabase::unreadMessagesForNumber);
    connect(this, &AsyncDatabase::requestLastMessageForNumber, this, &AsyncDatabase::lastMessageForNumber);
    connect(this, &AsyncDatabase::requestLastContactedForNumber, this, &AsyncDatabase::lastContactedForNumber);
    connect(this, &AsyncDatabase::requestMarkChatAsRead, this, &AsyncDatabase::markChatAsRead);
    connect(this, &AsyncDatabase::requestDeleteChat, this, &AsyncDatabase::deleteChat);
}

void AsyncDatabase::addMessage(const Message &message)
{
    m_database.addMessage(message);
}

void AsyncDatabase::messagesForNumber(const QString &phoneNumber)
{
    Q_EMIT messagesFetchedForNumber(phoneNumber, m_database.messagesForNumber(phoneNumber));
}

void AsyncDatabase::lastId()
{
    Q_EMIT lastIdFetched(m_database.lastId());
}

void AsyncDatabase::markMessageDelivered(const int id)
{
    m_database.markMessageDelivered(id);
}

void AsyncDatabase::markMessageRead(const int id)
{
    m_database.markMessageRead(id);
}

void AsyncDatabase::chats()
{
    Q_EMIT chatsFetched(m_database.chats());
}

void AsyncDatabase::unreadMessagesForNumber(const QString &phoneNumber)
{
    Q_EMIT unreadMessagesFetchedForNumber(phoneNumber, m_database.unreadMessagesForNumber(phoneNumber));
}

void AsyncDatabase::lastMessageForNumber(const QString &phoneNumber)
{
    Q_EMIT lastMessageFetchedForNumber(phoneNumber, m_database.lastMessageForNumber(phoneNumber));
}

void AsyncDatabase::lastContactedForNumber(const QString &phoneNumber)
{
    Q_EMIT lastContactedFetchedForNumber(phoneNumber, m_database.lastContactedForNumber(phoneNumber));
}

void AsyncDatabase::markChatAsRead(const QString &phoneNumber)
{
    m_database.markChatAsRead(phoneNumber);
}

void AsyncDatabase::deleteChat(const QString &phoneNumber)
{
    m_database.deleteChat(phoneNumber);
}
