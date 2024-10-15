// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <functional>

#include <QDBusArgument>
#include <QDebug>
#include <QSharedDataPointer>
#include <QString>

#include <global.h>

struct PhoneNumberPrivate;

///
/// Represents a phone number.
/// It can be efficiently converted into different phone number formats.
///
/// It is useful as an interface type, so the caller doesn't need to convert the phone number to any specific format.
///
class PhoneNumber
{
public:
    PhoneNumber();
    PhoneNumber(const PhoneNumber &other);
    ~PhoneNumber();
    PhoneNumber &operator=(const PhoneNumber &other);

    explicit PhoneNumber(const QString &number);

    bool operator==(const PhoneNumber &other) const;

    QString toInternational() const;
    QString toNational() const;
    QString toE164() const;
    bool isValid() const;
    static void setCountryCode(const QString &countryCode);

    QString toString() const;

private:
    QSharedDataPointer<PhoneNumberPrivate> d;
    static std::string countryCode;
};

inline QDebug &operator<<(QDebug &debug, const PhoneNumber &phoneNumber)
{
    return debug << phoneNumber.toInternational();
}

inline uint qHash(const PhoneNumber &phoneNum)
{
    return hash(phoneNum.toInternational());
}

Q_DECLARE_METATYPE(PhoneNumber)
