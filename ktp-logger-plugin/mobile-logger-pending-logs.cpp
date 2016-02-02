/*
 * Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "mobile-logger-pending-logs.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QDate>

#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/Account>

MobileLoggerPendingLogs::MobileLoggerPendingLogs(const Tp::AccountPtr &account, const KTp::LogEntity &entity,
                                                 const QDate &date, QObject *parent)
    : KTp::PendingLoggerLogs(account, entity, date, parent)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT message, isIncoming, messageDateTime, messageToken, contactData.targetContact, accountData.accountObjectPath FROM data "
                                 "INNER JOIN contactData ON data.targetContactId = contactData.id "
                                 "INNER JOIN accountData ON data.accountId = accountData.id "
                                 "WHERE contactData.targetContact = :entityId "
                                 "AND accountData.accountObjectPath = :accountObjectPath "
                                 "AND date(messageDateTime) = date(:date) "
                                 "ORDER BY messageDateTime ASC"));
    query.bindValue(QStringLiteral(":entityId"), entity.id());
    query.bindValue(QStringLiteral(":accountObjectPath"), account->objectPath());
    query.bindValue(QStringLiteral(":date"), date.toString(Qt::ISODate));
    query.exec();

    if (query.lastError().isValid()) {
        qWarning() << "SQL Error:" << query.lastError().text();
        setError(query.lastError().text());
        emitFinished();
        return;
    }


    QList<KTp::LogMessage> messages;
    KTp::LogEntity me = KTp::LogEntity(Tp::HandleTypeContact, QStringLiteral("me"), QStringLiteral("me"));
    KTp::LogEntity targetContact;

    while (query.next()) {
        if (query.value(1).toBool()) {
            if (!targetContact.isValid()) {
                targetContact = KTp::LogEntity(Tp::HandleTypeContact, query.value(4).toString(), QString());
            }
            messages << KTp::LogMessage(targetContact, account, QDateTime::fromString(query.value(2).toString(), Qt::ISODate),
                                        query.value(0).toString(), query.value(3).toString());
        }
    }

    qDebug() << messages.size();
    appendLogs(messages);
    emitFinished();
}
