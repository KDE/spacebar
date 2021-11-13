// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <unordered_set>

#include <global.h>

#include "phonenumber.h"

class PhoneNumberSet : public std::unordered_set<PhoneNumber>
{
public:
    using std::unordered_set<PhoneNumber>::unordered_set;

    explicit PhoneNumberSet(const QString &phoneNumbers);

    QString toString() const;
};

Q_DECLARE_METATYPE(PhoneNumberSet)
