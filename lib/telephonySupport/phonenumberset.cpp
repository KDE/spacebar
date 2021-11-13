// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumberset.h"

namespace ranges = std::ranges;

PhoneNumberSet::PhoneNumberSet(const QString &phoneNumbers)
    : std::unordered_set<PhoneNumber>()
{
    const auto individualNumbers = phoneNumbers.split(u';', Qt::SkipEmptyParts);

    ranges::transform(individualNumbers, std::inserter(*this, this->begin()), [](const QString &number) {
        return PhoneNumber(number);
    });
}

QString PhoneNumberSet::toString() const
{
    QStringList individualNumbers;
    individualNumbers.reserve(size());

    ranges::transform(*this, std::back_inserter(individualNumbers), [](const PhoneNumber &number) {
        return number.toInternational();
    });

    return individualNumbers.join(u';');
}
