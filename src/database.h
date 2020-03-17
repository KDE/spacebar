#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>

struct Message {
    QString phoneNumber;
    QString text;
    QDateTime time;
    bool read;
    bool sentByMe;
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
    QVector<Message> messagesForNumber(const QString &phoneNumber) const;

    // Chats
    QVector<Chat> chats() const;

private:
    QSqlDatabase m_database;

signals:
    void messagesChanged(const QString &phoneNumber);
};

#endif // DATABASE_H
