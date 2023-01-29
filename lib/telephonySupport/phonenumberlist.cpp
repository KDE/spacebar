// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumberlist.h"

namespace ranges = std::ranges;

PhoneNumberList::PhoneNumberList(const QString &phoneNumbers)
    : QVector<PhoneNumber>()
{
    const auto individualNumbers = phoneNumbers.split(u'~', Qt::SkipEmptyParts);
    reserve(individualNumbers.size());

    ranges::transform(individualNumbers, std::back_inserter(*this), [](const QString &number) {
        return PhoneNumber(number);
    });
}

QString PhoneNumberList::toString() const
{
    QStringList individualNumbers;
    individualNumbers.reserve(size());

    ranges::transform(*this, std::back_inserter(individualNumbers), [](const PhoneNumber &number) {
        return number.toInternational();
    });

    return individualNumbers.join(u'~');
}
