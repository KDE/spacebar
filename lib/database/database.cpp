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

#include <QCoroFuture>
#include <QCoroTask>
#include <random>

#include <global.h>
#include <phonenumberlist.h>

constexpr auto ID_LEN = 10;
constexpr auto DATABASE_REVISION = 8; // Keep MIGRATE_TO_LATEST_FROM in sync
#define MIGRATE_TO(n, current)                                                                                                                                 \
    if (current < n) {                                                                                                                                         \
        qDebug() << "Running migration" << #n;                                                                                                                 \
        migrationV##n(db, current);                                                                                                                            \
    }
#define MIGRATE_TO_LATEST_FROM(current) MIGRATE_TO(8, current)

template<typename T, typename Func>
std::optional<std::invoke_result_t<Func, T>> map(const std::optional<T> &&in, Func &&f)
{
    if (in.has_value()) {
        return f(std::move(*in));
    } else {
        return {};
    }
}

Database::Database(QObject *parent)
    : QObject(parent)
{
    Q_INIT_RESOURCE(migrations);

    const QString databaseLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar");
    if (!QDir().mkpath(databaseLocation)) {
        qDebug() << "Could not create the database directory at" << databaseLocation;
    }

    DatabaseConfiguration config;
    config.setDatabaseName(databaseLocation + SL("/messages.sqlite"));
    config.setType(DatabaseType::SQLite);
    m_database = ThreadedDatabase::establishConnection(config);
}

QFuture<std::vector<Message>> Database::messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id, const int limit) const
{
    QString sql = SL(R"(
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
            size,
            tapbacks
        FROM Messages
    )");

    if (id.isEmpty()) {
        sql.append(SL("WHERE phoneNumber == ? ORDER BY time DESC"));

        if (limit == 0) {
            sql.append(SL(" LIMIT 30"));
        } else if (limit == 1) {
            sql.append(SL(" LIMIT 1"));
        } else {
            sql.append(SL(" LIMIT -1 OFFSET 30"));
        }

        return m_database->getResults<Message>(sql, phoneNumberList.toString());
    } else {
        sql.append(SL("WHERE id == ?"));
        return m_database->getResults<Message>(sql, id);
    }
}

QFuture<void> Database::updateMessageDeliveryState(const QString &id, const MessageState state)
{
    return m_database->execute(SL("UPDATE Messages SET delivered = ? WHERE id == ?"), state, id);
}

QFuture<void> Database::updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation)
{
    return m_database->execute(SL("UPDATE Messages SET messageId = ?, contentLocation = ? WHERE id == ?"), messageId, contentLocation, id);
}

QFuture<void> Database::updateMessageDeliveryReport(const QString &messageId)
{
    return m_database->execute(SL("UPDATE Messages SET deliveryReport = IFNULL(deliveryReport, 0) + 1 WHERE messageId == ?"), messageId);
}

QFuture<void> Database::updateMessageReadReport(const QString &messageId, const PhoneNumber &fromNumber)
{
    return m_database->execute(SL("UPDATE Messages SET readReport = IFNULL(readReport, '') || ? WHERE messageId == ?"),
                               fromNumber.toInternational(),
                               messageId);
}

QFuture<void> Database::markMessageRead(const int id)
{
    return m_database->execute(SL("UPDATE Messages SET read = True WHERE id == ? AND NOT read = True"), id);
}

QFuture<void> Database::updateMessageTapbacks(const QString &id, const QString tapbacks)
{
    return m_database->execute(SL("UPDATE Messages SET tapbacks = ? WHERE id == ?"), tapbacks, id);
}

QCoro::Task<std::optional<QString>> Database::lastMessageWithText(const PhoneNumberList &phoneNumberList, const QString &text)
{
    auto id =
        co_await m_database->getResult<SingleValue<QString>>(SL("SELECT id FROM Messages WHERE phoneNumber == ? AND text == ? ORDER BY time DESC LIMIT 1"),
                                                             phoneNumberList.toString(),
                                                             text);
    co_return map(std::move(id), [](auto &&v) {
        return v.value;
    });
}

QCoro::Task<std::optional<QString>> Database::lastMessageWithAttachment(const PhoneNumberList &phoneNumberList)
{
    auto id = co_await m_database->getResult<SingleValue<QString>>(
        SL("SELECT id FROM Messages WHERE phoneNumber == ? AND attachments IS NOT NULL ORDER BY time DESC LIMIT 1"),
        phoneNumberList.toString());
    co_return map(std::move(id), [](auto &&v) {
        return v.value;
    });
}

QCoro::Task<QVector<Chat>> Database::chats(const PhoneNumberList &phoneNumberList) const
{
    QVector<Chat> chats;

    if (!phoneNumberList.empty()) {
        Chat chat;
        chat.phoneNumberList = phoneNumberList;
        chat.unreadMessages = (co_await unreadMessagesForNumber(chat.phoneNumberList)).value_or(0);

        std::vector<Message> messages = co_await messagesForNumber(chat.phoneNumberList, QString(), 1);

        if (!messages.empty()) {
            auto message = messages.front();
            chat.lastMessage = message.text;
            chat.lastDateTime = message.datetime;
            chat.lastSentByMe = message.sentByMe;
            chat.lastAttachment = message.attachments;
        }

        chats.push_back(std::move(chat));
    } else {
        auto numbers = co_await m_database->getResults<SingleValue<QString>>(SL(R"(
            WITH Numbers AS (
                SELECT
                    MAX(time) AS maxTime,
                    phoneNumber
                FROM Messages
                GROUP BY phoneNumber
            )
            SELECT phoneNumber
            FROM  Numbers
            ORDER BY maxTime desc
        )"));

        for (auto &&number : numbers) {
            Chat chat;
            chat.phoneNumberList = PhoneNumberList(number);
            chats.push_back(std::move(chat));
        }
    }

    co_return chats;
}

QCoro::Task<std::optional<int>> Database::unreadMessagesForNumber(const PhoneNumberList &phoneNumberList) const
{
    auto n = co_await m_database->getResult<SingleValue<int>>(SL("SELECT Count(*) FROM Messages WHERE phoneNumber == ? AND read == False"),
                                                              phoneNumberList.toString());
    co_return map(std::move(n), [](auto &&v) {
        return v.value;
    });
}

QFuture<void> Database::markChatAsRead(const PhoneNumberList &phoneNumberList)
{
    return m_database->execute(SL("UPDATE Messages SET read = True WHERE phoneNumber = ? AND NOT read == True"), phoneNumberList.toString());
}

QFuture<void> Database::deleteChat(const PhoneNumberList &phoneNumberList)
{
    return m_database->execute(SL("DELETE FROM Messages WHERE phoneNumber = ?"), phoneNumberList.toString());
}

QCoro::Task<> Database::addMessage(const Message &message)
{
    co_await m_database->execute(SL(R"(
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
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )"),
                                 message.id,
                                 message.phoneNumberList.toString(),
                                 message.text,
                                 message.datetime.toMSecsSinceEpoch(),
                                 message.read,
                                 message.deliveryStatus,
                                 message.sentByMe,
                                 message.attachments,
                                 message.smil,
                                 message.fromNumber,
                                 message.messageId,
                                 message.deliveryReport,
                                 message.readReport,
                                 message.pendingDownload,
                                 message.contentLocation,
                                 message.expires.isNull() ? QVariant() : message.expires.toMSecsSinceEpoch(),
                                 message.size);
}

QFuture<void> Database::deleteMessage(const QString &id)
{
    return m_database->execute(SL("DELETE FROM Messages WHERE id == ?"), id);
}

QString Database::generateRandomId()
{
    QString intermediateId = SL("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    std::shuffle(intermediateId.begin(), intermediateId.end(), std::mt19937(std::random_device()()));
    intermediateId.truncate(ID_LEN);

    return intermediateId;
}

QCoro::Task<> Database::mergeChats(const QString &fromNumbers, const QString toNumbers)
{
    co_await m_database->execute(SL("UPDATE Messages SET phoneNumber = ? WHERE phoneNumber = ?"), fromNumbers, toNumbers);

    // need to move files to correct chat attachment subdirectory
    QDir attachments(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments"));
    const QString folderFromHash = QString::number(hash(fromNumbers));
    const QString folderToHash = QString::number(hash(toNumbers));

    // if folder already exists, just move the files
    if (attachments.exists(folderToHash) && attachments.exists(folderFromHash)) {
        const QString folderPathFrom = attachments.path() + QDir::separator() + folderFromHash;
        const QString folderPathTo = attachments.path() + QDir::separator() + folderToHash;
        const QStringList files = QDir(folderPathFrom).entryList();
        for (auto &file : files) {
            QFile::copy(folderPathFrom + QDir::separator() + file, folderPathTo + QDir::separator() + file);
            QFile::remove(folderPathFrom + QDir::separator() + file);
        }
        attachments.rmdir(folderFromHash);
    } else if (attachments.exists(folderFromHash)) {
        attachments.rename(folderFromHash, folderToHash);
    }
}

QCoro::Task<> Database::migrate()
{
    // Check whether the database uses the old migration system
    auto revision = co_await m_database->runOnThread([](auto &db) -> std::optional<int> {
        // Find out current revision
        QSqlQuery currentRevision(db);
        currentRevision.prepare(SL("SELECT migrationId FROM Metadata ORDER BY migrationId DESC LIMIT 1"));
        exec(currentRevision);
        currentRevision.first();

        if (currentRevision.isValid()) {
            return currentRevision.value(0).toUInt();
        } else {
            return {};
        }
    });

    // If it does, first run all pending migrations of the old system
    if (revision) {
        qDebug() << "current (old) database revision" << *revision;
        m_database->runOnThread([this, revision = *revision](auto &db) {
            // Run migration if necessary
            if (revision >= DATABASE_REVISION) {
                return;
            }

            MIGRATE_TO_LATEST_FROM(revision);

            // Update migration info if necessary
            QSqlQuery update(db);
            update.prepare(SL("INSERT INTO Metadata (migrationId) VALUES (:migrationId)"));
            update.bindValue(SL(":migrationId"), DATABASE_REVISION);
            exec(update);
        });

        // Mark an equivalent migration level in the new system.
        m_database->setCurrentMigrationLevel(QStringLiteral("2023-05-07-102621_init"));
    }

    // Finally, run all migrations of the new system on top of that
    m_database->runMigrations(SL(":/migrations"));
}

void Database::exec(QSqlQuery &query)
{
    if (query.lastQuery().isEmpty()) {
        // Sending empty queries doesn't make sense
        Q_UNREACHABLE();
    }
    if (!query.exec()) {
        qWarning() << "Query" << query.lastQuery() << "resulted in" << query.lastError();
    }
}

void Database::migrationV1(const QSqlDatabase &db, uint)
{
    QSqlQuery createTable(db);
    createTable.prepare(
        SL("CREATE TABLE IF NOT EXISTS Messages (id INTEGER, phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered BOOLEAN, sentByMe BOOLEAN)"));
    Database::exec(createTable);
}

void Database::migrationV2(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(1, current);

    QSqlQuery tempTable(db);
    tempTable.prepare(SL("CREATE TABLE temp_table AS SELECT * FROM Messages"));
    Database::exec(tempTable);

    QSqlQuery dropOld(db);
    dropOld.prepare(SL("DROP TABLE Messages"));
    Database::exec(dropOld);

    QSqlQuery createNew(db);
    createNew.prepare(
        SL("CREATE TABLE IF NOT EXISTS Messages (id TEXT, phoneNumber TEXT, text TEXT, time DATETIME, read BOOLEAN, delivered INTEGER, sentByMe BOOLEAN)"));
    Database::exec(createNew);

    QSqlQuery copyTemp(db);
    copyTemp.prepare(SL("INSERT INTO Messages SELECT * FROM temp_table"));
    Database::exec(copyTemp);

    QSqlQuery dropTemp(db);
    dropTemp.prepare(SL("DROP TABLE temp_table"));
    Database::exec(dropTemp);
}

void Database::migrationV3(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(2, current);

    QSqlQuery getPhoneNumbers(db);
    getPhoneNumbers.prepare(SL("SELECT DISTINCT phoneNumber FROM Messages"));
    Database::exec(getPhoneNumbers);

    while (getPhoneNumbers.next()) {
        const auto phoneNumber = getPhoneNumbers.value(0).toString();
        qDebug() << "updating phone number" << phoneNumber;
        auto normalized = PhoneNumberList(phoneNumber).toString();
        qDebug() << "to" << normalized;

        QSqlQuery normalizePhoneNumbers(db);
        normalizePhoneNumbers.prepare(SL("UPDATE Messages SET phoneNumber = :normalized WHERE phoneNumber == :phoneNumber"));
        normalizePhoneNumbers.bindValue(SL(":normalized"), normalized);
        normalizePhoneNumbers.bindValue(SL(":phoneNumber"), phoneNumber);
        Database::exec(normalizePhoneNumbers);
    }
}

void Database::migrationV4(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(3, current);

    QSqlQuery addMmsColumns(db);
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

void Database::migrationV5(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(4, current);

    QSqlQuery fixDuplicateIds(db);
    fixDuplicateIds.prepare(SL("UPDATE Messages SET id = ROWID WHERE LENGTH(id) <> 10;"));
    Database::exec(fixDuplicateIds);
}

void Database::migrationV6(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(5, current);

    QSqlQuery removeHtml(db);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text,'&nbsp;', ' ')"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '&lt;', '<')"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '&gt;', '>')"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '&quot;', '\"')"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '&amp;', '&')"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '<br />', CHAR(13))"));
    Database::exec(removeHtml);
    removeHtml.prepare(SL("UPDATE Messages SET text = REPLACE(text, '</a>', '')"));
    Database::exec(removeHtml);
    removeHtml.prepare(
        SL("UPDATE Messages SET text = REPLACE(text, SUBSTR(text, INSTR(text, '<a href='), INSTR(text, '>http') - INSTR(text, '<a href=') + CASE WHEN "
           "INSTR(text, '>http') > 0 THEN 1 ELSE 0 END), '')"));
    Database::exec(removeHtml);
}

void Database::migrationV7(const QSqlDatabase &db, const uint current)
{
    MIGRATE_TO(6, current);

    QSqlQuery sql(db);
    sql.prepare(SL("ALTER TABLE Messages ADD COLUMN tapbacks TEXT"));
    Database::exec(sql);
}

void Database::migrationV8(const QSqlDatabase &db, uint current)
{
    MIGRATE_TO(7, current);

    QSqlQuery fetch(db);
    fetch.prepare(SL("SELECT DISTINCT phoneNumber FROM Messages"));
    exec(fetch);

    while (fetch.next()) {
        const QString original = fetch.value(0).toString();

        // now using the modem instead of locale to format numbers
        // fixes any numbers that were not formatted for cases where the locale was unset
        QString formatted = PhoneNumberList(fetch.value(0).toString().replace(u';', u'~')).toString();

        if (formatted.contains(u'~')) {
            QStringList numbers = formatted.split(u'~');
            numbers.sort();
            formatted = numbers.join(u'~');
        }

        if (original != formatted) {
            mergeChats(original, formatted);
        }
    }

    QSqlQuery sql(db);
    sql.prepare(SL("UPDATE Messages SET phoneNumber = REPLACE(phoneNumber,';', '~')"));
    Database::exec(sql);
}

Message Message::fromSql(ColumnTypes &&tuple)
{
    auto [id,
          phoneNumberList,
          text,
          datetime,
          read,
          deliveryStatus,
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
          size,
          tapbacks] = tuple;

    return Message{id,
                   PhoneNumberList(phoneNumberList),
                   text,
                   QDateTime::fromMSecsSinceEpoch(datetime),
                   read,
                   MessageState(deliveryStatus),
                   sentByMe,
                   attachments,
                   smil,
                   fromNumber,
                   messageId,
                   deliveryReport,
                   readReport,
                   pendingDownload,
                   contentLocation,
                   QDateTime::fromMSecsSinceEpoch(expires),
                   size,
                   tapbacks};
}
