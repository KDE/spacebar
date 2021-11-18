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
    qRegisterMetaType<MessageState>();

    // Forward messagesChanged signal
    connect(&m_database, &Database::messagesChanged, this, &AsyncDatabase::messagesChanged);

    // Connect requests to slots
    connect(this, &AsyncDatabase::requestAddMessage, this, &AsyncDatabase::addMessage);
    connect(this, &AsyncDatabase::requestDeleteMessage, this, &AsyncDatabase::deleteMessage);
    connect(this, &AsyncDatabase::requestUpdateMessageDeliveryState, this, &AsyncDatabase::updateMessageDeliveryState);
    connect(this, &AsyncDatabase::requestUpdateMessageSent, this, &AsyncDatabase::updateMessageSent);
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

void AsyncDatabase::deleteMessage(const QString &id)
{
    m_database.deleteMessage(id);
}

void AsyncDatabase::messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id)
{
    Q_EMIT messagesFetchedForNumber(phoneNumberList, m_database.messagesForNumber(phoneNumberList, id));
}

void AsyncDatabase::updateMessageDeliveryState(const QString &id, const MessageState state)
{
    m_database.updateMessageDeliveryState(id, state);
}

void AsyncDatabase::updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation)
{
    m_database.updateMessageSent(id, messageId, contentLocation);
}

void AsyncDatabase::markMessageRead(const int id)
{
    m_database.markMessageRead(id);
}

void AsyncDatabase::chats()
{
    Q_EMIT chatsFetched(m_database.chats());
}

void AsyncDatabase::unreadMessagesForNumber(const PhoneNumberList &phoneNumberList)
{
    Q_EMIT unreadMessagesFetchedForNumber(phoneNumberList, m_database.unreadMessagesForNumber(phoneNumberList));
}

void AsyncDatabase::lastMessageForNumber(const PhoneNumberList &phoneNumberList)
{
    Q_EMIT lastMessageFetchedForNumber(phoneNumberList, m_database.lastMessageForNumber(phoneNumberList));
}

void AsyncDatabase::lastContactedForNumber(const PhoneNumberList &phoneNumberList)
{
    Q_EMIT lastContactedFetchedForNumber(phoneNumberList, m_database.lastContactedForNumber(phoneNumberList));
}

void AsyncDatabase::markChatAsRead(const PhoneNumberList &phoneNumberList)
{
    m_database.markChatAsRead(phoneNumberList);
}

void AsyncDatabase::deleteChat(const PhoneNumberList &phoneNumberList)
{
    m_database.deleteChat(phoneNumberList);
}
