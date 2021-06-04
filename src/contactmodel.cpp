// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "contactmodel.h"

#include <global.h>

ContactModel::ContactModel(QObject *parent)
    : KPeople::PersonsSortFilterProxyModel(parent)
{
    setRequiredProperties({SL("phoneNumber")});
    setFilterRole(Qt::DisplayRole);
    setSortRole(Qt::DisplayRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    setSourceModel(new KPeople::PersonsModel(this));

    sort(0);
}
