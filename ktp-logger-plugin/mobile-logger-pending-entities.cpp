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

#include "mobile-logger-pending-entities.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDate>

#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/Account>

MobileLoggerPendingEntities::MobileLoggerPendingEntities(const Tp::AccountPtr &account,
                                                         QObject *parent)
    : KTp::PendingLoggerEntities(account, parent)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT targetContact FROM contactData "
                                 "INNER JOIN data ON data.targetContactId = contactData.id "
                                 "INNER JOIN accountData ON data.accountId = accountData.id "
                                 "AND accountData.accountObjectPath = :accountObjectPath "
                                 "GROUP BY targetContact"));
    query.bindValue(QStringLiteral(":accountObjectPath"), account->objectPath());
    query.exec();

    if (query.lastError().isValid()) {
        qWarning() << "SQL Error:" << query.lastError().text();
        setError(query.lastError().text());
        emitFinished();
        return;
    }

    QList<KTp::LogEntity> logEntities;

    while (query.next()) {
        logEntities << KTp::LogEntity(Tp::HandleTypeContact, query.value(0).toString(), QString());
    }

    appendEntities(logEntities);
    emitFinished();
}
