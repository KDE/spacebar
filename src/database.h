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

struct Chat {
    QString phoneNumber;
    QDateTime lastContacted;
    QString lastMessage;
    int unreadMessages;
};

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    // Messages
    void addMessage(const Message &message);
    void setMessageDelivered();
    QVector<Message> messagesForNumber(const QString &phoneNumber) const;
    int lastId() const;
    void markMessageDelivered(const int id);
    void markMessageRead(const int id);

    // Chats
    QVector<Chat> chats() const;
    int unreadMessagesForNumber(const QString &phoneNumber) const;
    QString lastMessageForNumber(const QString &phoneNumber) const;
    QDateTime lastContactedForNumber(const QString &phoneNumber) const;
    void markChatAsRead(const QString &phoneNumber);

private:
    QSqlDatabase m_database;

signals:
    void messagesChanged(const QString &phoneNumber);
};
