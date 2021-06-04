// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KPeople/PersonsSortFilterProxyModel>
#include <KPeople/PersonsModel>
#include <QSortFilterProxyModel>

///
/// The ContactModel class is a specialization of the KPeople Person model
/// which is sorted for the use with section delegates.
///
class ContactModel : public KPeople::PersonsSortFilterProxyModel
{
    Q_OBJECT

public:
    ContactModel(QObject *parent = nullptr);
};

