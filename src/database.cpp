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

    this->m_database.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + SL("/messages.sqlite"));
    bool open = this->m_database.open();

    if (!open) {
        qWarning() << "Could not open call database" << this->m_database.lastError();
    }

    QSqlQuery createTable(this->m_database);
    createTable.exec(SL("CREATE TABLE IF NOT EXISTS Messages (phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, sentByMe BOOLEAN)"));

    /*
    QString phoneNumber;
    QString text;
    QDateTime time;
    bool read;
    bool sentByMe;
    */
    this->addMessage({SL("+49 179 1281926"), SL("Nabend"), QDateTime::currentDateTime(), false, true});
    this->addMessage({SL("1751155152"), SL("Guten Abend"), QDateTime::currentDateTime(), false, true});
}

QVector<Message> Database::messagesForNumber(const QString &phoneNumber) const
{
    QVector<Message> messages;

    QSqlQuery fetch(this->m_database);
    fetch.prepare(SL("SELECT phoneNumber, text, time, read, sentByMe FROM Messages WHERE phoneNumber == :phoneNumber"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    while (fetch.next()) {
        Message message;
        message.phoneNumber = fetch.value(0).toString();
        message.text = fetch.value(1).toString();
        message.time = QDateTime::fromMSecsSinceEpoch(fetch.value(2).toInt());
        message.read = fetch.value(3).toBool();
        message.sentByMe = fetch.value(4).toBool();

        messages.append(message);
    }

    return messages;
}

QVector<Chat> Database::chats() const
{
    QVector<Chat> chats;

    QSqlQuery fetch(this->m_database);
    fetch.exec(SL("SELECT DISTINCT phoneNumber FROM Messages"));

    while (fetch.next()) {
        Chat chat;
        chat.phoneNumber = fetch.value(0).toString();
        chat.unreadMessages = this->unreadMessagesForNumber(chat.phoneNumber);
        chat.lastMessage = this->lastMessageForNumber(chat.phoneNumber);
        chat.lastContacted = this->lastContactedForNumber(chat.phoneNumber);
        qDebug() << chat.lastMessage;

        chats.append(chat);
    }

    return chats;
}

int Database::unreadMessagesForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(this->m_database);
    fetch.prepare(SL("SELECT Count(*) FROM Messages WHERE phoneNumber == :phoneNumber"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return fetch.value(0).toInt();
}

QString Database::lastMessageForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(this->m_database);
    fetch.prepare(SL("SELECT text FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return fetch.value(0).toString();
}

QDateTime Database::lastContactedForNumber(const QString &phoneNumber) const
{
    QSqlQuery fetch(this->m_database);
    fetch.prepare(SL("SELECT time FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    fetch.first();
    return QDateTime::fromMSecsSinceEpoch(fetch.value(0).toInt());
}

void Database::addMessage(const Message &message)
{
    QSqlQuery putCall(this->m_database);
    putCall.prepare(SL("INSERT INTO Messages (phoneNumber, text, time, read, sentByMe) VALUES (:phoneNumber, :text, :time, :read, :sentByMe)"));
    putCall.bindValue(SL(":phoneNumber"), message.phoneNumber);
    putCall.bindValue(SL(":text"), message.text);
    putCall.bindValue(SL(":time"), message.time.toMSecsSinceEpoch());
    putCall.bindValue(SL(":read"), message.read);
    putCall.bindValue(SL(":sentByMe"), message.sentByMe);
    putCall.exec();

    emit messagesChanged(message.phoneNumber);
}
