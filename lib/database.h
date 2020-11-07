// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>

struct Message {
    int id;
    QString phoneNumber;
    QString text;
    QDateTime datetime;
    bool read;
    bool sentByMe;
    bool delivered;
};
Q_DECLARE_METATYPE(Message);

struct Chat {
    QString phoneNumber;
    QDateTime lastContacted;
    QString lastMessage;
    int unreadMessages;
};
Q_DECLARE_METATYPE(Chat);

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

private:
    // Messages
    Q_SLOT void addMessage(const Message &message);
    Q_SLOT void messagesForNumber(const QString &phoneNumber);
    Q_SLOT void lastId();
    Q_SLOT void markMessageDelivered(const int id);
    Q_SLOT void markMessageRead(const int id);

    // Chats
    Q_SLOT void chats();
    Q_SLOT int unreadMessagesForNumber(const QString &phoneNumber);
    Q_SLOT QString lastMessageForNumber(const QString &phoneNumber);
    Q_SLOT QDateTime lastContactedForNumber(const QString &phoneNumber) const;
    Q_SLOT void markChatAsRead(const QString &phoneNumber);

    QSqlDatabase m_database;

signals:
    void messagesChanged(const QString &phoneNumber);

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
    Q_SIGNAL void requestMarkAsRead(const QString &phoneNumber);

    // Responses
    void messagesFetchedForNumber(const QString &phoneNumber, const QVector<Message> messages);
    void lastIdFetched(const int id);
    void chatsFetched(QVector<Chat> chats);
    void unreadMessagesFetchedForNumber(const QString &phoneNumber, const int unreadMessages);
    void lastMessageFetchedForNumber(const QString &phoneNumber, const QString &message);
    void lastContactedFetchedForNumber(const QString &phoneNumber, const QDateTime &lastContacted);
};
