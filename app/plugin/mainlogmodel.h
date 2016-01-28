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

#ifndef MAINLOGMODEL_H
#define MAINLOGMODEL_H

#include <QObject>
#include <QIdentityProxyModel>

class QSqlQueryModel;

class MainLogModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    enum Role {
        ContactDisplayName = Qt::DisplayRole,
        ContactIdRole = Qt::UserRole,
        LastMessageDate,
        LastMessageText,

        UserRole = Qt::UserRole + 0x1000 ///< in case it's needed to extend, use this one to start from
    };
    Q_ENUMS(Role)

    MainLogModel(QObject *parent = 0);
    ~MainLogModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
//     virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
//     virtual int columnCount(const QModelIndex &index = QModelIndex()) const Q_DECL_OVERRIDE;

    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

private:
    QSqlQueryModel *m_dbModel;


};

#endif
