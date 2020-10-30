// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "global.h"

enum Column {
    Id = 0,
    PhoneNumber,
    Text,
    DateTime,
    Read,
    Delivered,
    SentByMe
};

Database::Database(QObject *parent)
    : QObject(parent)
    , m_database(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("messages")))
{
    const auto databaseLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar");
    if (!QDir().mkpath(databaseLocation)) {
        qDebug() << "Could not create the database directory at" << databaseLocation;
    }

    m_database.setDatabaseName(databaseLocation + SL("/messages.sqlite"));
    bool open = m_database.open();

    if (!open) {
        qWarning() << "Could not open call database" << m_database.lastError();
    }

    QSqlQuery createTable(m_database);
    createTable.exec(SL("CREATE TABLE IF NOT EXISTS Messages (id INTEGER, phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered BOOLEAN, sentByMe BOOLEAN)"));
}

QVector<Message> Database::messagesForNumber(const QString &phoneNumber) const
{
    QVector<Message> messages;

    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT id, phoneNumber, text, time, read, delivered, sentByMe FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY id DESC"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumber);
    fetch.exec();

    while (fetch.next()) {
        Message message;
        message.id = fetch.value(Column::Id).toInt();
        message.phoneNumber = fetch.value(Column::PhoneNumber).toString();
        message.text = fetch.value(Column::Text).toString();
        message.datetime = QDateTime::fromMSecsSinceEpoch(fetch.value(Column::DateTime).value<quint64>());
        message.read = fetch.value(Column::Read).toBool();
        message.delivered = fetch.value(Column::Delivered).toBool();
        message.sentByMe = fetch.value(Column::SentByMe).toBool();

        messages.append(message);
    }

    return messages;
}

int Database::lastId() const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT id FROM Messages ORDER BY id DESC LIMIT 1"));
    fetch.exec();
    fetch.first();

    return fetch.value(0).toInt();
}

void Database::markMessageDelivered(const int id)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET delivered = True WHERE id == :id AND NOT delivered = True"));
    put.bindValue(SL(":id"), id);
    put.exec();
}

void Database::markMessageRead(const int id)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET read = True WHERE id == :id AND NOT read = True"));
    put.bindValue(SL(":id"), id);
    put.exec();
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
}

void Database::addMessage(const Message &message)
{
    auto before = QTime::currentTime().msecsSinceStartOfDay();
    QSqlQuery putCall(m_database);
    putCall.prepare(SL("INSERT INTO Messages (id, phoneNumber, text, time, read, delivered, sentByMe) VALUES (:id, :phoneNumber, :text, :time, :read, :delivered, :sentByMe)"));
    putCall.bindValue(SL(":id"), message.id);
    putCall.bindValue(SL(":phoneNumber"), message.phoneNumber);
    putCall.bindValue(SL(":text"), message.text);
    putCall.bindValue(SL(":time"), message.datetime.toMSecsSinceEpoch());
    putCall.bindValue(SL(":read"), message.read);
    putCall.bindValue(SL(":sentByMe"), message.sentByMe);
    putCall.bindValue(SL(":delivered"), message.delivered);
    putCall.exec();

    qDebug() << "WRITING TOOK TIME" << QTime::currentTime().msecsSinceStartOfDay() - before;
}
