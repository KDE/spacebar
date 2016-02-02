/*
 * Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org
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

#include "mobile-logger-pending-dates.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QDate>

#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/Account>

MobileLoggerPendingDates::MobileLoggerPendingDates(const Tp::AccountPtr &account, const KTp::LogEntity &entity, QObject *parent)
    : KTp::PendingLoggerDates(account, entity, parent)
{
//     QSqlDatabase db = ;

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT messageDateTime FROM data INNER JOIN contactData ON data.targetContactId = contactData.id "
                                 "INNER JOIN accountData ON data.accountId = accountData.id "
                                 "WHERE contactData.targetContact = :entityId "
                                 "AND accountData.accountObjectPath = :accountObjectPath"));
    query.bindValue(QStringLiteral(":entityId"), entity.id());
    query.bindValue(QStringLiteral(":accountObjectPath"), account->objectPath());
    query.exec();

    if (query.lastError().isValid()) {
        qDebug() << "Error:" << query.lastError().text();
        setError(query.lastError().text());
        emitFinished();
        return;
    }

    QSet<QDate> dates;
    while (query.next()) {
        qDebug() << QDate::fromString(query.value(0).toString(), Qt::ISODate);
        dates << QDate::fromString(query.value(0).toString(), Qt::ISODate);
    }


    setDates(dates.toList());
    emitFinished();
}
