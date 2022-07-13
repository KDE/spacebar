// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "asyncdatabase.h"


AsyncDatabase::AsyncDatabase()
{
    qRegisterMetaType<Message>();
    qRegisterMetaType<Chat>();
    qRegisterMetaType<QVector<Chat>>();
    qRegisterMetaType<QVector<Message>>();
    qRegisterMetaType<MessageState>();

    // Forward messagesChanged signal
    connect(&m_database, &Database::messagesChanged, this, &AsyncDatabase::messagesChanged);
}

QFuture<void> AsyncDatabase::addMessage(const Message &message)
{
    return runAsync<void>([=, this] { m_database.addMessage(message); });
}

QFuture<void> AsyncDatabase::deleteMessage(const QString &id)
{
    return runAsync<void>([=, this] { m_database.deleteMessage(id); });
}

QFuture<QVector<Message>> AsyncDatabase::messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id)
{
    return runAsync<QVector<Message>>([=, this] { return m_database.messagesForNumber(phoneNumberList, id); });
}

QFuture<void> AsyncDatabase::updateMessageDeliveryState(const QString &id, const MessageState state)
{
    return runAsync<void>([=, this] { m_database.updateMessageDeliveryState(id, state); });
}

QFuture<void> AsyncDatabase::updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation)
{
    return runAsync<void>([=, this] { m_database.updateMessageSent(id, messageId, contentLocation); });
}

QFuture<void> AsyncDatabase::markMessageRead(const int id)
{
    return runAsync<void>([=, this] { m_database.markMessageRead(id); });
}

QFuture<QVector<Chat>> AsyncDatabase::chats()
{
    return runAsync<QVector<Chat>>([=, this] { return  m_database.chats(); });
}

QFuture<void> AsyncDatabase::markChatAsRead(const PhoneNumberList &phoneNumberList)
{
    return runAsync<void>([=, this] { m_database.markChatAsRead(phoneNumberList); });
}

QFuture<void> AsyncDatabase::deleteChat(const PhoneNumberList &phoneNumberList)
{
    return runAsync<void>([=, this] { m_database.deleteChat(phoneNumberList); });
}
