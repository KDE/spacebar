// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumberutils.h"

#include <QLocale>

#include <phonenumbers/phonenumberutil.h>


using namespace ::i18n::phonenumbers;

namespace phoneNumberUtils {

auto countryCode() {
    const QLocale locale;
    const QStringList qcountry = locale.name().split(u'_');
    const QString &countrycode(qcountry.constLast());
    return countrycode.toStdString();
}

NormalizeResult normalizeNumber(const std::string &numberString)
{
    static auto country = countryCode();

    PhoneNumber phoneNumber;
    auto error = PhoneNumberUtil::GetInstance()->Parse(numberString, country, &phoneNumber);

    if (error != PhoneNumberUtil::NO_PARSING_ERROR) {
        return ErrorType(error);
    }

    std::string formattedNumber;
    PhoneNumberUtil::GetInstance()->Format(phoneNumber, PhoneNumberUtil::INTERNATIONAL, &formattedNumber);
    return formattedNumber;
}

QString normalizeNumber(const QString &numberString)
{
    auto res = normalizeNumber(numberString.toStdString());
    if (std::holds_alternative<std::string>(res)) {
        return QString::fromStdString(std::get<std::string>(res));
    }

    return numberString;
}

}
