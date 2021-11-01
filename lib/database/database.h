// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <global.h>
#include <phonenumber.h>


enum MessageState {
    Unknown = false,
    Sent = true,
    Pending,
    Failed,
    Received
};
Q_DECLARE_METATYPE(MessageState);

inline MessageState parseMessageState(const QString &state) {
    if (state == SL("pending")) {
        return MessageState::Pending;
    } else if (state == SL("sent")) {
        return MessageState::Sent;
    } else if (state == SL("failed")) {
        return MessageState::Failed;
    }

    Q_UNREACHABLE();
};

struct Message {
    QString id;
    PhoneNumber phoneNumber;
    QString text;
    QDateTime datetime;
    bool read;
    bool sentByMe;
    MessageState deliveryStatus;
};
Q_DECLARE_METATYPE(Message);

struct Chat {
    PhoneNumber phoneNumber;
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

    // Messages
    void addMessage(const Message &message);
    void deleteMessage(const QString &id);
    QVector<Message> messagesForNumber(const PhoneNumber &phoneNumber) const;
    void updateMessageDeliveryState(const QString &id, const MessageState state);
    void markMessageRead(const int id);

    // Chats
    QVector<Chat> chats() const;
    int unreadMessagesForNumber(const PhoneNumber &phoneNumber) const;
    QString lastMessageForNumber(const PhoneNumber &phoneNumber) const;
    QDateTime lastContactedForNumber(const PhoneNumber &phoneNumber) const;
    void markChatAsRead(const PhoneNumber &phoneNumber);
    void deleteChat(const PhoneNumber &phoneNumber);

    static QString generateRandomId();

    static void exec(QSqlQuery &query);

private:
    void migrationV1(uint current);
    void migrationV2(uint current);
    void migrationV3(uint current);
    void migrate();

    QSqlDatabase m_database;

signals:
    void messagesChanged(const PhoneNumber &phoneNumber);
};
