// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QVector>

#include <global.h>

#include "phonenumber.h"

class PhoneNumberList : public QVector<PhoneNumber>
{
public:
    using QVector<PhoneNumber>::QVector;

    explicit PhoneNumberList(const QString &phoneNumbers);

    QString toString() const;
};

Q_DECLARE_METATYPE(PhoneNumberList)
