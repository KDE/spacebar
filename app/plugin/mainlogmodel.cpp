/*
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mainlogmodel.h"

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QStandardPaths>

#include <QDebug>

MainLogModel::MainLogModel(QObject *parent)
    : QIdentityProxyModel(parent),
      m_dbModel(new QSqlQueryModel(this))
{
    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktp-mobile-logger/");

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    qDebug() << dbLocation << db.open();

    m_dbModel->setQuery(QStringLiteral("SELECT contactData.targetContact, data.messageDateTime, data.message FROM data LEFT JOIN contactData ON data.targetContactId = contactData.id GROUP BY data.targetContactId ORDER BY data.messageDateTime DESC"));

    setSourceModel(m_dbModel);
}

MainLogModel::~MainLogModel()
{

}

QVariant MainLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    qDebug() << m_dbModel->record(index.row()) << m_dbModel->record(index.row()).value("targetContact");
    qDebug() << index << index.row() << index.column();

    switch (role) {
        case MainLogModel::ContactDisplayName:
        case MainLogModel::ContactIdRole:
            return m_dbModel->record(index.row()).value("targetContact");
        case MainLogModel::LastMessageDate:
            return m_dbModel->record(index.row()).value("messageDateTime");
        case MainLogModel::LastMessageText:
            return m_dbModel->record(index.row()).value("message");
    }

    return QVariant();
}

// int MainLogModel::rowCount(const QModelIndex &index) const
// {
// //     qDebug() << "humm";
//     return m_dbModel->rowCount(index);
// }

QHash<int, QByteArray> MainLogModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    roles.insert(ContactDisplayName, "contactDisplayName");
    roles.insert(ContactIdRole, "contactIdRole");
    roles.insert(LastMessageDate, "lastMessageDate");
    roles.insert(LastMessageText, "lastMessageText");

    return roles;
}
