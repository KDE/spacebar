// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "utils.h"

#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QProcess>

#include "global.h"

Utils *Utils::s_instance = nullptr;


Utils::Utils(QQmlApplicationEngine *engine)
    : m_engine(engine)

{
    s_instance = this;
}

void Utils::showPassiveNotification(const QString &message, int timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    if (m_window) {
        QMetaObject::invokeMethod(m_window, "showPassiveNotification", Q_ARG(QVariant, message), Q_ARG(QVariant, timeout), Q_ARG(QVariant, {}), Q_ARG(QVariant, {}));
    }
}

void Utils::showPassiveNotification(const QString &message, Utils::PassiveNotificationDuation timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    QString timeoutStr;
    switch(timeout) {
    case ShortNotificationDuration:
        timeoutStr = SL("short");
        break;
    case LongNotificationDuration:
        timeoutStr = SL("long");
        break;
    }
    if (m_window) {
        QMetaObject::invokeMethod(m_window, "showPassiveNotification", Q_ARG(QVariant, message), Q_ARG(QVariant, timeoutStr), Q_ARG(QVariant, {}), Q_ARG(QVariant, {}));
    }
}

bool Utils::isPhoneNumber(const QString &text)
{
    if (m_phoneNumberRegex.pattern().isEmpty()) {
        qDebug() << "compiling regex";
        m_phoneNumberRegex = QRegularExpression(SL(R"(^[+]*[(]{0,1}[0-9]{1,4}[)]{0,1}[-\s\./0-9]*$)"));
        m_phoneNumberRegex.optimize();
    }

    return m_phoneNumberRegex.match(text).hasMatch();
}

void Utils::launchPhonebook()
{
    QProcess::startDetached(SL("plasma-phonebook"), {});
}

Utils *Utils::instance()
{
    return s_instance;
}

QQmlApplicationEngine *Utils::qmlEngine() const
{
    return m_engine;
}
