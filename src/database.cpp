#include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "global.h"

Database::Database(QObject *parent)
    : QObject(parent)
    , m_database(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("messages")))
{
    if (!QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))) {
        qDebug() << "Could not create the database directory at" << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }

    m_database.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + SL("/messages.sqlite"));
    bool open = m_database.open();

    if (!open) {
        qWarning() << "Could not open call database" << m_database.lastError();
    }

    QSqlQuery createTable(m_database);
    createTable.exec(SL("CREATE TABLE IF NOT EXISTS Messages (phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered BOOLEAN, sentByMe BOOLEAN)"));

    /*
    struct Message {
    QString phoneNumber;
    QString text;
    QDateTime time;
    bool read;
    bool sentByMe;
    };*/

    addMessage({SL("4234"), SL("Halloooooo"), QDateTime::currentDateTime(), false, true, true});
}

QVector<Message> Database::messagesForNumber(const QString &phoneNumber) const
{
    QVector<Message> messages;

    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT phoneNumber, text, time, read, delivered, sentByMe FROM Messages WHERE phoneNumber == :phoneNumber"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    qDebug() << fetch.lastError();

    while (fetch.next()) {
        Message message;
        message.phoneNumber = fetch.value(0).toString();
        message.text = fetch.value(1).toString();
        message.time = QDateTime::fromMSecsSinceEpoch(fetch.value(2).toInt());
        message.read = fetch.value(3).toBool();
        message.delivered = fetch.value(4).toBool();
        message.sentByMe = fetch.value(5).toBool();

        messages.append(message);
    }

    return messages;
}

QVector<Chat> Database::chats() const
{
    QVector<Chat> chats;

    auto before = QTime::currentTime().msecsSinceStartOfDay();

    QSqlQuery fetch(m_database);
    fetch.exec(SL("SELECT DISTINCT phoneNumber FROM Messages"));

    while (fetch.next()) {
        Chat chat;
        chat.phoneNumber = fetch.value(0).toString();
        chat.unreadMessages = unreadMessagesForNumber(chat.phoneNumber);
        chat.lastMessage = lastMessageForNumber(chat.phoneNumber);
        chat.lastContacted = lastContactedForNumber(chat.phoneNumber);

        chats.append(chat);
    }

    auto after = QTime::currentTime().msecsSinceStartOfDay();
    qDebug() << "TOOK TIME" << after - before;

    return chats;
}

int Database::unreadMessagesForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT Count(*) FROM Messages WHERE phoneNumber == :phoneNumber AND read == False"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return fetch.value(0).toInt();
}

QString Database::lastMessageForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT text FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return fetch.value(0).toString();
}

QDateTime Database::lastContactedForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT time FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return QDateTime::fromMSecsSinceEpoch(fetch.value(0).toInt());
}

void Database::markChatAsRead(const QString &phoneNumber)
{
    QSqlQuery update(m_database);
    update.prepare(SL("UPDATE Messages SET read = True WHERE phoneNumber = :phoneNumber AND NOT read == True"));
    update.bindValue(SL(":phoneNumber"), phoneNumber);
    update.exec();

    emit messagesChanged(phoneNumber);
}

void Database::addMessage(const Message &message)
{
    //auto before = QTime::currentTime().msecsSinceStartOfDay();
    QSqlQuery putCall(m_database);
    putCall.prepare(SL("INSERT INTO Messages (phoneNumber, text, time, read, delivered, sentByMe) VALUES (:phoneNumber, :text, :time, :read, :delivered, :sentByMe)"));
    putCall.bindValue(SL(":phoneNumber"), message.phoneNumber);
    putCall.bindValue(SL(":text"), message.text);
    putCall.bindValue(SL(":time"), message.time.toMSecsSinceEpoch());
    putCall.bindValue(SL(":read"), message.read);
    putCall.bindValue(SL(":sentByMe"), message.sentByMe);
    putCall.bindValue(SL(":delivered"), message.delivered);
    putCall.exec();

    //qDebug() << "WRITING TOOK TIME" << QTime::currentTime().msecsSinceStartOfDay() - before;

    emit messagesChanged(message.phoneNumber);
}
