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

#include "mobile-logger.h"
#include "mobile-logger-pending-dates.h"
#include "mobile-logger-pending-entities.h"
#include "mobile-logger-pending-logs.h"

#include <KTp/Logger/pending-logger-search.h>

#include <QDebug>
#include <QSqlDatabase>

#include <KPluginFactory>

class A : public KTp::PendingLoggerSearch
{
    Q_OBJECT

public:
    A(const QString &term, QObject *parent = 0) : KTp::PendingLoggerSearch(term, parent)
    {

    }
};

KTp::MobileLoggerPlugin::MobileLoggerPlugin(QObject *parent, const QVariantList &)
    : KTp::AbstractLoggerPlugin(parent)
{
    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktp-mobile-logger/");

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    qDebug() << dbLocation << db.open();
}

KTp::MobileLoggerPlugin::~MobileLoggerPlugin()
{

}

KTp::PendingLoggerDates* KTp::MobileLoggerPlugin::queryDates(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{
    return new MobileLoggerPendingDates(account, entity, this);
}

KTp::PendingLoggerLogs* KTp::MobileLoggerPlugin::queryLogs(const Tp::AccountPtr &account, const KTp::LogEntity &entity, const QDate &date)
{
    return new MobileLoggerPendingLogs(account, entity, date, this);
}

KTp::PendingLoggerEntities* KTp::MobileLoggerPlugin::queryEntities(const Tp::AccountPtr &account)
{
    return new MobileLoggerPendingEntities(account, this);
}

bool KTp::MobileLoggerPlugin::handlesAccount(const Tp::AccountPtr &account)
{
    return true;
}

void KTp::MobileLoggerPlugin::clearAccountLogs(const Tp::AccountPtr &account)
{

}

void KTp::MobileLoggerPlugin::clearContactLogs(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{

}

KTp::PendingLoggerSearch* KTp::MobileLoggerPlugin::search(const QString &term)
{
    return new A(term);
}

bool KTp::MobileLoggerPlugin::logsExist(const Tp::AccountPtr &account, const KTp::LogEntity &contact)
{
    return true;
}

K_PLUGIN_FACTORY(MobileLoggerPluginFactory, registerPlugin<KTp::MobileLoggerPlugin>();)

#include "mobile-logger.moc"
