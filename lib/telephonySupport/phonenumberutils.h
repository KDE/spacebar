// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

class QString;

class PhoneNumberUtils {
public:
    static QString normalize(const QString &phoneNumber);
};
