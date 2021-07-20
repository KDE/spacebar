// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "phonenumberutils.h"

#include <QChar>
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

NormalizeResult normalizeNumber(const std::string &numberString, PhoneNumberFormat format)
{
    static auto country = countryCode();

    PhoneNumber phoneNumber;
    auto error = PhoneNumberUtil::GetInstance()->Parse(numberString, country, &phoneNumber);

    if (error != PhoneNumberUtil::NO_PARSING_ERROR) {
        return ErrorType(error);
    }

    std::string formattedNumber;
    PhoneNumberUtil::GetInstance()->Format(phoneNumber, PhoneNumberUtil::PhoneNumberFormat(format), &formattedNumber);
    return formattedNumber;
}

QString normalizeNumber(const QString &numberString, PhoneNumberFormat format)
{
    auto res = normalizeNumber(numberString.toStdString(), format);
    if (std::holds_alternative<std::string>(res)) {
        return QString::fromStdString(std::get<std::string>(res));
    }

    return numberString;
}

// normalize a number so that Ofono can understand it
QString normalizeForOfono(const QString &numberString)
{
    // ofono number parsing:
    // https://github.com/rilmodem/ofono/blob/efc9c0a85d32706bc088e449e847be41dcc73b3d/src/common.c#L238

    QString normalized;
    for (int i = 0; i < numberString.size(); ++i) {
        QChar c = numberString.at(i);
        if (c == QChar(u'+') && normalized.isEmpty()) {
            // plus is allowed but only as first character
            normalized.append(u'+');
        } else if ((c >= QChar(u'0') && c <= QChar(u'9')) || c == QChar(u'*') ||
                   c == QChar(u'#')) {
            normalized.append(c);
        }
        // ignore all other characters
    }

    return normalized;
}
}
