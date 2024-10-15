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

// For use with DBus
typedef QMap<QString, QString> StringMap;
typedef QList<StringMap> StringMapList;
Q_DECLARE_METATYPE(StringMap)
Q_DECLARE_METATYPE(StringMapList)

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

    Message() = default;

    Message(const StringMap &map)
    {
        id = map[SL("id")];
        phoneNumberList = PhoneNumberList{map[SL("phoneNumberList")]};
        text = map[SL("text")];
        datetime = QDateTime::fromString(map[SL("datetime")]);
        read = QVariant{map[SL("read")]}.toBool();
        deliveryStatus = static_cast<MessageState>(map[SL("deliveryStatus")].toUInt());
        sentByMe = QVariant{map[SL("sentByMe")]}.toBool();
        attachments = map[SL("attachments")];
        smil = map[SL("smil")];
        fromNumber = map[SL("fromNumber")];
        messageId = map[SL("messageId")];
        deliveryReport = map[SL("deliveryReport")].toInt();
        readReport = map[SL("readReport")];
        pendingDownload = QVariant{map[SL("pendingDownload")]}.toBool();
        contentLocation = map[SL("contentLocation")];
        expires = QDateTime::fromString(map[SL("expires")]);
        size = map[SL("size")].toInt();
        tapbacks = map[SL("tapbacks")];
    }

    StringMap toMap() const
    {
        return {
            {SL("id"), id},
            {SL("phoneNumberList"), phoneNumberList.toString()},
            {SL("text"), text},
            {SL("datetime"), datetime.toString()},
            {SL("read"), QString::number(read)},
            {SL("deliveryStatus"), QString::number(deliveryStatus)},
            {SL("sentByMe"), QString::number(sentByMe)},
            {SL("attachments"), attachments},
            {SL("smil"), smil},
            {SL("fromNumber"), fromNumber},
            {SL("messageId"), messageId},
            {SL("deliveryReport"), QString::number(deliveryReport)},
            {SL("readReport"), readReport},
            {SL("pendingDownload"), QString::number(pendingDownload)},
            {SL("contentLocation"), contentLocation},
            {SL("expires"), expires.toString()},
            {SL("size"), QString::number(size)},
            {SL("tapbacks"), tapbacks},
        };
    }
};
Q_DECLARE_METATYPE(Message)

struct Chat {
    PhoneNumberList phoneNumberList;
    int unreadMessages = 0;
    QString lastMessage;
    QDateTime lastDateTime;
    bool lastSentByMe = false;
    QString lastAttachment;

    Chat() = default;

    Chat(const StringMap &map)
    {
        phoneNumberList = PhoneNumberList{map[SL("phoneNumberList")]};
        unreadMessages = map[SL("unreadMessages")].toInt();
        lastMessage = map[SL("lastMessage")];
        lastDateTime = QDateTime::fromString(map[SL("lastDateTime")]);
        lastSentByMe = QVariant{map[SL("lastSentByMe")]}.toBool();
        lastAttachment = map[SL("lastAttachment")];
    }

    StringMap toMap() const
    {
        return {{SL("phoneNumberList"), phoneNumberList.toString()},
                {SL("unreadMessages"), QString::number(unreadMessages)},
                {SL("lastMessage"), lastMessage},
                {SL("lastDateTime"), lastDateTime.toString()},
                {SL("lastSentByMe"), QString::number(lastSentByMe)},
                {SL("lastAttachment"), lastAttachment}};
    }
};

Q_DECLARE_METATYPE(Chat)

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);

    // Messages
    QCoro::Task<> addMessage(const Message &message);
    QCoro::Task<> deleteMessage(const QString &id);
    QCoro::Task<std::vector<Message>> messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id = QString(), const int limit = 0) const;
    QCoro::Task<> updateMessageDeliveryState(const QString &id, const MessageState state);
    QCoro::Task<> updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    QCoro::Task<> updateMessageDeliveryReport(const QString &messageId);
    QCoro::Task<> updateMessageReadReport(const QString &messageId, const PhoneNumber &fromNumber);
    QCoro::Task<> markMessageRead(const int id);
    QCoro::Task<> updateMessageTapbacks(const QString &id, const QString tapbacks);
    QCoro::Task<std::optional<QString>> lastMessageWithText(const PhoneNumberList &phoneNumberList, const QString &text);
    QCoro::Task<std::optional<QString>> lastMessageWithAttachment(const PhoneNumberList &phoneNumberList);

    // Chats
    QCoro::Task<QVector<Chat>> chats(const PhoneNumberList &phoneNumberList) const;
    QCoro::Task<std::optional<int>> unreadMessagesForNumber(const PhoneNumberList &phoneNumberList) const;
    QCoro::Task<> markChatAsRead(const PhoneNumberList &phoneNumberList);
    QCoro::Task<> deleteChat(const PhoneNumberList &phoneNumberList);
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
