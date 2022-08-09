// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <global.h>
#include <phonenumberlist.h>


enum MessageState {
    Unknown = false,
    Sent = true,
    Pending,
    Failed,
    Received
};
Q_DECLARE_METATYPE(MessageState)

inline MessageState parseMessageState(const QString &state) {
    if (state == SL("pending")) {
        return MessageState::Pending;
    } else if (state == SL("sent")) {
        return MessageState::Sent;
    } else if (state == SL("failed")) {
        return MessageState::Failed;
    }

    Q_UNREACHABLE();
}

struct Message {
    QString id;
    PhoneNumberList phoneNumberList;
    QString text;
    QDateTime datetime;
    bool read;
    bool sentByMe;
    MessageState deliveryStatus;
    QString attachments;
    QString smil;
    QString fromNumber;
    QString messageId;
    int deliveryReport = 0;
    QString readReport;
    bool pendingDownload = false;
    QString contentLocation;
    QDateTime expires;
    int size = 0;
    QString tapbacks;
};
Q_DECLARE_METATYPE(Message)

struct Chat {
    PhoneNumberList phoneNumberList;
    QDateTime lastContacted;
    QString lastMessage;
    int unreadMessages;
    bool lastSentByMe;
    QString lastAttachment;
};
Q_DECLARE_METATYPE(Chat)

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    // Messages
    void addMessage(const Message &message);
    void deleteMessage(const QString &id);
    QVector<Message> messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id = QString(), const bool last = false) const;
    void updateMessageDeliveryState(const QString &id, const MessageState state);
    void updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    void updateMessageDeliveryReport(const QString &messageId);
    void updateMessageReadReport(const QString &messageId, const PhoneNumber &fromNumber);
    void markMessageRead(const int id);
    void updateMessageTapbacks(const QString &id, const QString tapbacks);
    QString lastMessageWithText(const PhoneNumberList &phoneNumberList, const QString &text);
    QString lastMessageWithAttachment(const PhoneNumberList &phoneNumberList);

    // Chats
    QVector<Chat> chats() const;
    int unreadMessagesForNumber(const PhoneNumberList &phoneNumberList) const;
    void markChatAsRead(const PhoneNumberList &phoneNumberList);
    void deleteChat(const PhoneNumberList &phoneNumberList);

    static QString generateRandomId();

    static void exec(QSqlQuery &query);

private:
    void migrationV1(uint current);
    void migrationV2(uint current);
    void migrationV3(uint current);
    void migrationV4(uint current);
    void migrationV5(uint current);
    void migrationV6(uint current);
    void migrationV7(uint current);
    void migrate();

    QSqlDatabase m_database;

Q_SIGNALS:
    void messagesChanged(const PhoneNumberList &phoneNumberList);
};
