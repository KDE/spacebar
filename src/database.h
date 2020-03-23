#pragma once

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>

struct Message {
    QString phoneNumber;
    QString text;
    QDateTime time;
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
