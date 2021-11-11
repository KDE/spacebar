// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "asyncdatabase.h"

#include <QFuture>

#include <memory>

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
    return invokeOnThread<void>([=, this] {
        return m_database.addMessage(message);
    });
}

QFuture<void> AsyncDatabase::deleteMessage(const QString &id)
{
    return invokeOnThread<void>([=, this] {
        m_database.deleteMessage(id);
    });
}

QFuture<QVector<Message>> AsyncDatabase::messagesForNumber(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<QVector<Message>>([=, this] {
        return m_database.messagesForNumber(phoneNumber);
    });
}

QFuture<void> AsyncDatabase::updateMessageDeliveryState(const QString &id, const MessageState state)
{
    return invokeOnThread<void>([=, this] {
        return m_database.updateMessageDeliveryState(id, state);
    });
}

QFuture<void> AsyncDatabase::markMessageRead(const int id)
{
    return invokeOnThread<void>([=, this] {
        return m_database.markMessageRead(id);
    });
}

QFuture<QVector<Chat>> AsyncDatabase::chats()
{
    qDebug() << "Fetching chats 1";
    return invokeOnThread<QVector<Chat>>([=, this] {
        qDebug() << "Fetching chats 2";
        return m_database.chats();
    });
}

QFuture<int> AsyncDatabase::unreadMessagesForNumber(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<int>([=, this] {
        return m_database.unreadMessagesForNumber(phoneNumber);
    });
}

QFuture<QString> AsyncDatabase::lastMessageForNumber(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<QString>([=, this] {
        return m_database.lastMessageForNumber(phoneNumber);
    });
}

QFuture<QDateTime> AsyncDatabase::lastContactedForNumber(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<QDateTime>([=, this] {
        return m_database.lastContactedForNumber(phoneNumber);
    });
}

QFuture<void> AsyncDatabase::markChatAsRead(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<void>([=, this] {
        return m_database.markChatAsRead(phoneNumber);
    });
}

QFuture<void> AsyncDatabase::deleteChat(const PhoneNumber &phoneNumber)
{
    return invokeOnThread<void>([=, this] {
        return m_database.deleteChat(phoneNumber);
    });
}
