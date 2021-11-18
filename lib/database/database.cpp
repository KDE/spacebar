// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include <random>

#include <phonenumberlist.h>
#include <global.h>

constexpr auto ID_LEN = 10;
constexpr auto DATABASE_REVISION = 4; // Keep MIGRATE_TO_LATEST_FROM in sync
#define MIGRATE_TO(n, current) \
    if (current < n) { \
        qDebug() << "Running migration" << #n; \
        migrationV##n(current); \
    }
#define MIGRATE_TO_LATEST_FROM(current) MIGRATE_TO(4, current)

enum Column {
    IdColumn = 0,
    PhoneNumberColumn,
    TextColumn,
    DateTimeColumn,
    ReadColumn,
    DeliveryStateColumn,
    SentByMeColumn,
    AttachmentsColumn,
    SmilColumn,
    FromNumberColumn,
    MessageIdColumn,
    DeliveryReportColumn,
    ReadReportColumn,
    PendingDownloadColumn,
    ContentLocationColumn,
    ExpiresColumn,
    SizeColumn
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
    const bool open = m_database.open();

    if (!open) {
        qWarning() << "Could not open messages database" << m_database.lastError();
    }

    migrate();
}

QVector<Message> Database::messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id) const
{
    QVector<Message> messages;

    QSqlQuery fetch(m_database);
    fetch.prepare(SL(R"(
        SELECT
            id,
            phoneNumber,
            text,
            time,
            read,
            delivered,
            sentByMe,
            attachments,
            smil,
            fromNumber,
            messageId,
            deliveryReport,
            readReport,
            pendingDownload,
            contentLocation,
            expires,
            size
        FROM Messages
        WHERE (phoneNumber == :phoneNumber AND :id IS NULL) OR id ==:id
        ORDER BY time DESC
    )"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    fetch.bindValue(SL(":id"), id);
    exec(fetch);

    while (fetch.next()) {
        Message message;
        message.id = fetch.value(Column::IdColumn).toString();
        message.phoneNumberList = phoneNumberList;
        message.text = fetch.value(Column::TextColumn).toString();
        message.datetime = QDateTime::fromMSecsSinceEpoch(fetch.value(Column::DateTimeColumn).value<quint64>());
        message.read = fetch.value(Column::ReadColumn).toBool();
        message.deliveryStatus = fetch.value(Column::DeliveryStateColumn).value<MessageState>();
        message.sentByMe = fetch.value(Column::SentByMeColumn).toBool();
        message.attachments = fetch.value(Column::AttachmentsColumn).toString();
        message.smil = fetch.value(Column::SmilColumn).toString();
        message.fromNumber = fetch.value(Column::FromNumberColumn).toString();
        message.messageId = fetch.value(Column::MessageIdColumn).toString();
        message.deliveryReport = fetch.value(Column::DeliveryReportColumn).toInt();
        message.readReport = fetch.value(Column::ReadReportColumn).toString();
        message.pendingDownload = fetch.value(Column::PendingDownloadColumn).toBool();
        message.contentLocation = fetch.value(Column::ContentLocationColumn).toString();
        message.expires = QDateTime::fromMSecsSinceEpoch(fetch.value(Column::ExpiresColumn).value<quint64>());
        message.size = fetch.value(Column::SizeColumn).toInt();

        messages.append(std::move(message));
    }

    return messages;
}

void Database::updateMessageDeliveryState(const QString &id, const MessageState state)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET delivered = :state WHERE id == :id"));
    put.bindValue(SL(":id"), id);
    put.bindValue(SL(":state"), state);
    exec(put);
}

void Database::updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET messageId = :messageId, contentLocation = :contentLocation WHERE id == :id"));
    put.bindValue(SL(":id"), id);
    put.bindValue(SL(":messageId"), messageId);
    put.bindValue(SL(":contentLocation"), contentLocation);
    exec(put);
}

void Database::updateMessageDeliveryReport(const QString &messageId)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET deliveryReport = IFNULL(deliveryReport, 0) + 1 WHERE messageId == :messageId"));
    put.bindValue(SL(":messageId"), messageId);
    exec(put);
}

void Database::updateMessageReadReport(const QString &messageId, const PhoneNumber &fromNumber)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET readReport = IFNULL(readReport, '') || :fromNumber WHERE messageId == :messageId"));
    put.bindValue(SL(":messageId"), messageId);
    put.bindValue(SL(":fromNumber"), fromNumber.toInternational());
    exec(put);
}

void Database::markMessageRead(const int id)
{
    QSqlQuery put(m_database);
    put.prepare(SL("UPDATE Messages SET read = True WHERE id == :id AND NOT read = True"));
    put.bindValue(SL(":id"), id);
    exec(put);
}

QVector<Chat> Database::chats() const
{
    QVector<Chat> chats;

    auto before = QTime::currentTime().msecsSinceStartOfDay();

    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT DISTINCT phoneNumber FROM Messages"));
    exec(fetch);

    while (fetch.next()) {
        Chat chat;
        chat.phoneNumberList = PhoneNumberList(fetch.value(0).toString());
        chat.unreadMessages = unreadMessagesForNumber(chat.phoneNumberList);
        chat.lastMessage = lastMessageForNumber(chat.phoneNumberList);
        chat.lastContacted = lastContactedForNumber(chat.phoneNumberList);

        chats.append(chat);
    }

    auto after = QTime::currentTime().msecsSinceStartOfDay();
    qDebug() << "TOOK TIME" << after - before;

    return chats;
}

int Database::unreadMessagesForNumber(const PhoneNumberList &phoneNumberList) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT Count(*) FROM Messages WHERE phoneNumber == :phoneNumber AND read == False"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    exec(fetch);

    fetch.first();
    return fetch.value(0).toInt();
}

QString Database::lastMessageForNumber(const PhoneNumberList &phoneNumberList) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT text FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    fetch.exec();

    fetch.first();
    return fetch.value(0).toString();
}

QDateTime Database::lastContactedForNumber(const PhoneNumberList &phoneNumberList) const
{
    QSqlQuery fetch(m_database);
    fetch.prepare(SL("SELECT time FROM Messages WHERE phoneNumber == :phoneNumber ORDER BY time DESC LIMIT 1"));
    fetch.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    exec(fetch);

    fetch.first();
    return QDateTime::fromMSecsSinceEpoch(fetch.value(0).toLongLong());
}

void Database::markChatAsRead(const PhoneNumberList &phoneNumberList)
{
    QSqlQuery update(m_database);
    update.prepare(SL("UPDATE Messages SET read = True WHERE phoneNumber = :phoneNumber AND NOT read == True"));
    update.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    exec(update);

    Q_EMIT messagesChanged(phoneNumberList);
}

void Database::deleteChat(const PhoneNumberList &phoneNumberList)
{
    QSqlQuery update(m_database);
    update.prepare(SL("DELETE FROM Messages WHERE phoneNumber = :phoneNumber"));
    update.bindValue(SL(":phoneNumber"), phoneNumberList.toString());
    exec(update);

    Q_EMIT messagesChanged(phoneNumberList);
}

void Database::addMessage(const Message &message)
{
    QSqlQuery putCall(m_database);
    putCall.prepare(SL(R"(
        INSERT INTO Messages (
            id,
            phoneNumber,
            text,
            time,
            read,
            delivered,
            sentByMe,
            attachments,
            smil,
            fromNumber,
            messageId,
            deliveryReport,
            readReport,
            pendingDownload,
            contentLocation,
            expires,
            size)
        VALUES (
            :id,
            :phoneNumber,
            :text,
            :time,
            :read,
            :delivered,
            :sentByMe,
            :attachments,
            :smil,
            :fromNumber,
            :messageId,
            :deliveryReport,
            :readReport,
            :pendingDownload,
            :contentLocation,
            :expires,
            :size)
        )"));
    putCall.bindValue(SL(":id"), message.id);
    putCall.bindValue(SL(":phoneNumber"), message.phoneNumberList.toString());
    putCall.bindValue(SL(":text"), message.text);
    putCall.bindValue(SL(":time"), message.datetime.toMSecsSinceEpoch());
    putCall.bindValue(SL(":read"), message.read);
    putCall.bindValue(SL(":delivered"), message.deliveryStatus);
    putCall.bindValue(SL(":sentByMe"), message.sentByMe);
    putCall.bindValue(SL(":attachments"), message.attachments);
    putCall.bindValue(SL(":smil"), message.smil);
    putCall.bindValue(SL(":fromNumber"), message.fromNumber);
    putCall.bindValue(SL(":messageId"), message.messageId);
    putCall.bindValue(SL(":deliveryReport"), message.deliveryReport);
    putCall.bindValue(SL(":readReport"), message.readReport);
    putCall.bindValue(SL(":pendingDownload"), message.pendingDownload);
    putCall.bindValue(SL(":contentLocation"), message.contentLocation);
    if (!message.expires.isNull()) {
        putCall.bindValue(SL(":expires"), message.expires.toMSecsSinceEpoch());
    }
    putCall.bindValue(SL(":size"), message.size);
    exec(putCall);

    Q_EMIT messagesChanged(message.phoneNumberList);
}

void Database::deleteMessage(const QString &id)
{
    QSqlQuery del(m_database);
    del.prepare(SL("DELETE FROM Messages WHERE id == :id"));
    del.bindValue(SL(":id"), id);
    exec(del);
}

QString Database::generateRandomId()
{
    QString intermediateId = SL("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    std::shuffle(intermediateId.begin(), intermediateId.end(), std::mt19937(std::random_device()()));
    intermediateId.truncate(ID_LEN);

    return intermediateId;
}

void Database::migrate()
{
    // Create migration table if necessary
    QSqlQuery createMetadata(m_database);
    createMetadata.prepare(SL("CREATE TABLE IF NOT EXISTS Metadata (migrationId INTEGER NOT NULL)"));
    exec(createMetadata);

    // Find out current revision
    QSqlQuery currentRevision(m_database);
    currentRevision.prepare(SL("SELECT migrationId FROM Metadata ORDER BY migrationId DESC LIMIT 1"));
    exec(currentRevision);
    currentRevision.first();

    uint revision = 0;
    if (currentRevision.isValid()) {
         revision = currentRevision.value(0).toUInt();
    }

    qDebug() << "current database revision" << revision;

    // Run migration if necessary
    if (revision >= DATABASE_REVISION) {
        return;
    }

    MIGRATE_TO_LATEST_FROM(revision);

    // Update migration info if necessary
    QSqlQuery update(m_database);
    update.prepare(SL("INSERT INTO Metadata (migrationId) VALUES (:migrationId)"));
    update.bindValue(SL(":migrationId"), DATABASE_REVISION);
    exec(update);
}

void Database::exec(QSqlQuery &query)
{
    if (query.lastQuery().isEmpty()) {
        // Sending empty queries doesn't make sense
        Q_UNREACHABLE();
    }
    if (!query.exec()) {
        qWarning() <<  "Query" << query.lastQuery() << "resulted in" << query.lastError();
    }
}

void Database::migrationV1(uint)
{
    QSqlQuery createTable(m_database);
    createTable.prepare(SL("CREATE TABLE IF NOT EXISTS Messages (id INTEGER, phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered BOOLEAN, sentByMe BOOLEAN)"));
    Database::exec(createTable);
}

void Database::migrationV2(uint current)
{
    MIGRATE_TO(1, current);

    QSqlQuery tempTable(m_database);
    tempTable.prepare(SL("CREATE TABLE temp_table AS SELECT * FROM Messages"));
    Database::exec(tempTable);

    QSqlQuery dropOld(m_database);
    dropOld.prepare(SL("DROP TABLE Messages"));
    Database::exec(dropOld);

    QSqlQuery createNew(m_database);
    createNew.prepare(SL("CREATE TABLE IF NOT EXISTS Messages (id TEXT, phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered INTEGER, sentByMe BOOLEAN)"));
    Database::exec(createNew);

    QSqlQuery copyTemp(m_database);
    copyTemp.prepare(SL("INSERT INTO Messages SELECT * FROM temp_table"));
    Database::exec(copyTemp);

    QSqlQuery dropTemp(m_database);
    dropTemp.prepare(SL("DROP TABLE temp_table"));
    Database::exec(dropTemp);
}

void Database::migrationV3(uint current)
{
    MIGRATE_TO(2, current);

    QSqlQuery getPhoneNumbers(m_database);
    getPhoneNumbers.prepare(SL("SELECT DISTINCT phoneNumber FROM Messages"));
    Database::exec(getPhoneNumbers);

    while (getPhoneNumbers.next()) {
        const auto phoneNumber = getPhoneNumbers.value(0).toString();
        qDebug() << "updating phone number" << phoneNumber;
        auto normalized = PhoneNumberList(phoneNumber).toString();
        qDebug() << "to" << normalized;

        QSqlQuery normalizePhoneNumbers(m_database);
        normalizePhoneNumbers.prepare(SL("UPDATE Messages SET phoneNumber = :normalized WHERE phoneNumber == :phoneNumber"));
        normalizePhoneNumbers.bindValue(SL(":normalized"), normalized);
        normalizePhoneNumbers.bindValue(SL(":phoneNumber"), phoneNumber);
        Database::exec(normalizePhoneNumbers);
    }
}

void Database::migrationV4(uint current)
{
    MIGRATE_TO(3, current);

    QSqlQuery addMmsColumns(m_database);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN attachments TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN smil TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN fromNumber TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN messageId TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN deliveryReport INTEGER;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN readReport TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN pendingDownload BOOLEAN;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN contentLocation TEXT;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN expires DATETIME;"));
    Database::exec(addMmsColumns);
    addMmsColumns.prepare(SL("ALTER TABLE Messages ADD COLUMN size INTEGER;"));
    Database::exec(addMmsColumns);
}
