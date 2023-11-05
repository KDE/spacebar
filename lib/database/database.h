// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <ThreadedDatabase>

#include <QCoroTask>

#include <global.h>
#include <phonenumberlist.h>

enum MessageState {
    Unknown = false,
    Sent = true,
    Pending,
    Failed,
    Received,
};
Q_DECLARE_METATYPE(MessageState)

inline MessageState parseMessageState(const QString &state)
{
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
    using ColumnTypes =
        std::tuple<QString, QString, QString, qint64, bool, int, bool, QString, QString, QString, QString, int, QString, bool, QString, qint64, int, QString>;

    static Message fromSql(ColumnTypes &&tuple);

    QString id;
    PhoneNumberList phoneNumberList;
    QString text;
    QDateTime datetime;
    bool read;
    MessageState deliveryStatus;
    bool sentByMe;
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
    int unreadMessages = 0;
    QString lastMessage;
    QDateTime lastDateTime;
    bool lastSentByMe = false;
    QString lastAttachment;
};
Q_DECLARE_METATYPE(Chat)

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    // Messages
    QCoro::Task<> addMessage(const Message &message);
    QFuture<void> deleteMessage(const QString &id);
    QFuture<std::vector<Message>> messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id = QString(), const int limit = 0) const;
    QFuture<void> updateMessageDeliveryState(const QString &id, const MessageState state);
    QFuture<void> updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    QFuture<void> updateMessageDeliveryReport(const QString &messageId);
    QFuture<void> updateMessageReadReport(const QString &messageId, const PhoneNumber &fromNumber);
    QFuture<void> markMessageRead(const int id);
    QFuture<void> updateMessageTapbacks(const QString &id, const QString tapbacks);
    QCoro::Task<std::optional<QString>> lastMessageWithText(const PhoneNumberList &phoneNumberList, const QString &text);
    QCoro::Task<std::optional<QString>> lastMessageWithAttachment(const PhoneNumberList &phoneNumberList);

    // Chats
    QCoro::Task<QVector<Chat>> chats(const PhoneNumberList &phoneNumberList) const;
    QCoro::Task<std::optional<int>> unreadMessagesForNumber(const PhoneNumberList &phoneNumberList) const;
    QFuture<void> markChatAsRead(const PhoneNumberList &phoneNumberList);
    QFuture<void> deleteChat(const PhoneNumberList &phoneNumberList);
    QCoro::Task<> mergeChats(const QString &fromNumbers, const QString toNumbers);

    static QString generateRandomId();

    static void exec(QSqlQuery &query);

    QCoro::Task<> migrate();

private:
    // Ran on the database thread
    void migrationV1(const QSqlDatabase &db, uint current);
    void migrationV2(const QSqlDatabase &db, uint current);
    void migrationV3(const QSqlDatabase &db, uint current);
    void migrationV4(const QSqlDatabase &db, uint current);
    void migrationV5(const QSqlDatabase &db, uint current);
    void migrationV6(const QSqlDatabase &db, uint current);
    void migrationV7(const QSqlDatabase &db, uint current);
    void migrationV8(const QSqlDatabase &db, uint current);

    std::unique_ptr<ThreadedDatabase> m_database;
};
