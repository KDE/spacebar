// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumberutils.h"

#include <KContacts/PhoneNumber>

QString PhoneNumberUtils::normalize(const QString &phoneNumber)
{
    const QString normalized = KContacts::PhoneNumber(phoneNumber).normalizedNumber();
    // "Number" is probably a string, which should be kept intact
    if (normalized.isEmpty()) {
        return phoneNumber;
    } else {
        return normalized;
    }
}
