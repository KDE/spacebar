// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QRegularExpression>

#include <KContacts/PhoneNumber>
#include <KContacts/VCardConverter>

#include <phonenumberlist.h>

class QQmlApplicationEngine;
class QQuickWindow;

class Utils : public QObject
{
    Q_OBJECT

public:
    enum PassiveNotificationDuation {
        ShortNotificationDuration,
        LongNotificationDuration,
    };
    Q_ENUM(PassiveNotificationDuation)

    explicit Utils(QQmlApplicationEngine *engine);
    void showPassiveNotification(const QString &message, int timeout = 0);
    void showPassiveNotification(const QString &message, PassiveNotificationDuation timeout);

    Q_INVOKABLE bool isPhoneNumber(const QString &text) const;
    Q_INVOKABLE bool isPremiumNumber(const PhoneNumberList &phoneNumberList) const;
    Q_INVOKABLE static void launchPhonebook();
    Q_INVOKABLE void copyTextToClipboard(const QString &text) const;
    Q_INVOKABLE PhoneNumber phoneNumber(const QString &number) const;
    Q_INVOKABLE PhoneNumberList phoneNumberList(const QStringList &phoneNumbers) const;

    /// Currently only used to normalize contact model. Please try not to use this.
    Q_INVOKABLE QString phoneNumberToInternationalString(const PhoneNumber &phoneNumber) const;

    /// Currently only used to add a phone number to an image provider uri.
    /// Please try not to use this.
    Q_INVOKABLE QString phoneNumberListToString(const PhoneNumberList &phoneNumberList) const;

    bool isLocale24HourTime();

    Q_INVOKABLE static QString textToHtml(const QString &text);

    Q_INVOKABLE QVariantList phoneNumbers(const QString &kPeopleUri);

    static Utils *instance();

    static Utils *s_instance;

    QQmlApplicationEngine *qmlEngine() const;

private:
    QQmlApplicationEngine *m_engine;
    QQuickWindow *m_window = nullptr;
    const KContacts::VCardConverter converter;
};
