// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumber.h"

#include <phonenumbers/phonenumberutil.h>

#include <QLocale>
#include <QDebug>

using namespace i18n;

struct PhoneNumberPrivate : QSharedData {
    i18n::phonenumbers::PhoneNumber number;
};

std::string countryCode() {
    const QLocale locale;
    const QStringList qcountry = locale.name().split(u'_');
    const QString &countrycode(qcountry.constLast());
    return countrycode.toStdString();
}

PhoneNumber::PhoneNumber()
    : d(new PhoneNumberPrivate())
{
}
PhoneNumber::~PhoneNumber() = default;
PhoneNumber::PhoneNumber(const PhoneNumber &other) = default;
PhoneNumber &PhoneNumber::operator=(const PhoneNumber &other) = default;

PhoneNumber::PhoneNumber(const QString &number)
    : PhoneNumber()
{
    static auto country = countryCode();

    auto error = phonenumbers::PhoneNumberUtil::GetInstance()->Parse(number.toStdString(), country, &d->number);
    Q_UNUSED(error); // TODO
}


bool PhoneNumber::operator==(const PhoneNumber &other) const
{
    if (!isValid()) {
        return false;
    }
    return phonenumbers::PhoneNumberUtil::GetInstance()->IsNumberMatch(d->number, other.d->number);
}

QString PhoneNumber::toInternational() const
{
    std::string formattedNumber;
    phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::INTERNATIONAL, &formattedNumber);
    return QString::fromStdString(formattedNumber);
}

QString PhoneNumber::toNational() const
{
    std::string formattedNumber;
    phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::NATIONAL, &formattedNumber);
    return QString::fromStdString(formattedNumber);
}

QString PhoneNumber::toE164() const
{
    std::string formattedNumber;
    phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::E164, &formattedNumber);
    return QString::fromStdString(formattedNumber);
}

bool PhoneNumber::isValid() const
{
    return d->number.IsInitialized();
}
