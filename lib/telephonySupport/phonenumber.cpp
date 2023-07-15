// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumber.h"

#include <phonenumbers/phonenumberutil.h>

using namespace i18n;

struct PhoneNumberPrivate : QSharedData {
    enum Representation {
        Parsed,
        String,
    };

    i18n::phonenumbers::PhoneNumber number;
    QString numberString;
    Representation representation;
};

std::string PhoneNumber::countryCode;

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
    auto error = phonenumbers::PhoneNumberUtil::GetInstance()->Parse(number.toStdString(), countryCode, &d->number);
    if (error == phonenumbers::PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        d->representation = PhoneNumberPrivate::Parsed;
    } else {
        d->numberString = number;
        d->representation = PhoneNumberPrivate::String;
    }
}

bool PhoneNumber::operator==(const PhoneNumber &other) const
{
    if (!isValid()) {
        return false;
    }
    if (d->representation == PhoneNumberPrivate::Parsed && other.d->representation == PhoneNumberPrivate::Parsed) {
        return phonenumbers::PhoneNumberUtil::GetInstance()->IsNumberMatch(d->number, other.d->number) == phonenumbers::PhoneNumberUtil::EXACT_MATCH;
    } else if (d->representation == PhoneNumberPrivate::String && other.d->representation == PhoneNumberPrivate::String) {
        return d->numberString == other.d->numberString;
    }

    // The same input cannot be successfully parsed one time and the other time not, so assume false here
    return false;
}

QString PhoneNumber::toInternational() const
{
    if (d->representation == PhoneNumberPrivate::Parsed) {
        std::string formattedNumber;
        phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::INTERNATIONAL, &formattedNumber);
        return QString::fromStdString(formattedNumber);
    } else {
        return d->numberString;
    }
}

QString PhoneNumber::toNational() const
{
    if (d->representation == PhoneNumberPrivate::Parsed) {
        std::string formattedNumber;
        phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::NATIONAL, &formattedNumber);
        return QString::fromStdString(formattedNumber);
    } else {
        return d->numberString;
    }
}

QString PhoneNumber::toE164() const
{
    if (d->representation == PhoneNumberPrivate::Parsed) {
        std::string formattedNumber;
        phonenumbers::PhoneNumberUtil::GetInstance()->Format(d->number, phonenumbers::PhoneNumberUtil::PhoneNumberFormat::E164, &formattedNumber);
        return QString::fromStdString(formattedNumber);
    } else {
        return d->numberString;
    }
}

bool PhoneNumber::isValid() const
{
    if (d->representation == PhoneNumberPrivate::Parsed) {
        return d->number.IsInitialized();
    } else {
        return !d->numberString.isEmpty();
    }
}

void PhoneNumber::setCountryCode(const QString &code)
{
    countryCode = code.toStdString();
}
