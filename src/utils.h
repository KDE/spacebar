// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

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

    Q_INVOKABLE bool isPhoneNumber(const QString &text);
    Q_INVOKABLE static void launchPhonebook();

    static Utils *instance();

    static Utils *s_instance;

    QQmlApplicationEngine *qmlEngine() const;

private:
    QQmlApplicationEngine *m_engine;
    QQuickWindow *m_window = nullptr;
    QRegularExpression m_phoneNumberRegex;
};
