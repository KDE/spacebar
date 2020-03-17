#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QSqlError>
#include <QDebug>
#include <QDir>

#include "Global.h"

Database::Database(QObject *parent)
    : QObject(parent)
    , m_database(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("messages")))
{
    if (!QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))) {
        qDebug() << "Could not create the database directory at" << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }

    qDebug() << "path" << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    m_database.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + SL("/messages.sqlite"));
    bool open = m_database.open();

    if (!open) {
        qWarning() << "Could not open call database" << m_database.lastError();
    }

    QSqlQuery createTable(m_database);
    createTable.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS Messages (phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, sentByMe BOOLEAN)"));

    /*
    QString phoneNumber;
    QString text;
    QDateTime time;
    bool read;
    bool sentByMe;
    */
    addMessage({SL("12345"), SL("Hey"), {}, false, true});
}

QVector<Message> Database::messagesForNumber(const QString &phoneNumber) const
{
    QVector<Message> messages;

    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT phoneNumber, text, time, read, sentByMe, FROM Messages WHERE phoneNumber == :phoneNumber"));
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

    QSqlQuery fetch(m_database);
    fetch.exec(SL("SELECT DISTINCT phoneNumber FROM Messages"));


    while (fetch.next()) {
        Chat chat;
        chat.phoneNumber = fetch.value(0).toString();

        chats.append(chat);
    }

    return chats;
}

void Database::addMessage(const Message &message)
{
    QSqlQuery putCall(m_database);
    putCall.prepare(SL("INSERT INTO Messages (phoneNumber, text, time, read, sentByMe) VALUES (:phoneNumber, :text, :time, :read, :sentByMe)"));
    putCall.bindValue(SL(":phoneNumber"), message.phoneNumber);
    putCall.bindValue(SL(":text"), message.text);
    putCall.bindValue(SL(":time"), message.time);
    putCall.bindValue(SL(":read"), message.read);
    putCall.bindValue(SL(":sentByMe"), message.sentByMe);
    putCall.exec();

    emit messagesChanged(message.phoneNumber);
}
