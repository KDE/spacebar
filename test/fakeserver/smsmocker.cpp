// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "smsmocker.h"
#include "smsadaptor.h"

#include <QDBusConnection>

SmsMocker::SmsMocker(QObject *parent, QString number, QString text, QString timestamp, QString id, int state)
    : QObject{parent}
    , m_number{number}
    , m_text{text}
    , m_timestamp{timestamp}
    , m_state{state}
{
    setDBusName(QStringLiteral("org.freedesktop.ModemManager1.Sms"));
    setDBusPath(QDBusObjectPath(QStringLiteral("/org/freedesktop/ModemManager1/Modem/0/Messages/") + id));
}

void SmsMocker::registerObject()
{
    if (!m_initialized) {
        qDebug() << "Initializing dbus mock for org.freedesktop.ModemManager1.Sms...";

        new SmsAdaptor{this};

        auto bus = QDBusConnection::systemBus();
        bool status = bus.registerObject(dbusPath().path(), this);

        if (status) {
            m_initialized = true;
        } else {
            qDebug() << "Failed to init!";
        }
    }
}

QString SmsMocker::number() const
{
    return m_number;
}

QString SmsMocker::text() const
{
    return m_text;
}

QString SmsMocker::timestamp() const
{
    return m_timestamp;
}

int SmsMocker::state() const
{
    return m_state;
}
