// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <string>
#include <optional>
#include <variant>

class QString;

namespace phoneNumberUtils {
enum ErrorType {
      NoParsingError,
      InvalidCountryCodeError,
      NotANumber,
      TooShortAfterIID,
      TooShortNSN,
      TooLongNsn,
};

enum PhoneNumberFormat {
      E164,
      International,
      National,
      RFC3966
};

using NormalizeResult = std::variant<std::string, ErrorType>;

NormalizeResult normalizeNumber(const std::string &numberString, PhoneNumberFormat format = International);
QString normalizeNumber(const QString &numberString, PhoneNumberFormat format = International);
};
