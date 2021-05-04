// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QRegularExpression>
class QQmlApplicationEngine;
class QQuickWindow;

class Utils : public QObject
{
    Q_OBJECT

public:
    enum PassiveNotificationDuation {
        ShortNotificationDuration,
        LongNotificationDuration
    };
    Q_ENUM(PassiveNotificationDuation)

    explicit Utils(QQmlApplicationEngine *engine);
    void showPassiveNotification(const QString &message, int timeout = 0);
    void showPassiveNotification(const QString &message, PassiveNotificationDuation timeout);

    Q_INVOKABLE bool isPhoneNumber(const QString &text) const;
    Q_INVOKABLE bool isPremiumNumber(const QString &text) const;
    Q_INVOKABLE static void launchPhonebook();

    static QString textToHtml(const QString &text);

    static Utils *instance();

    static Utils *s_instance;

    QQmlApplicationEngine *qmlEngine() const;

private:
    QQmlApplicationEngine *m_engine;
    QQuickWindow *m_window = nullptr;
};
