// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "modemmocker.h"
#include "modemadaptor.h"

#include <QDBusConnection>

ModemMocker::ModemMocker(QObject *parent)
    : QObject{parent}
{
    setDBusName(QStringLiteral("org.freedesktop.ModemManager1.Modem"));
    setDBusPath(QDBusObjectPath(QStringLiteral("/org/freedesktop/ModemManager1/Modem/0")));
}

void ModemMocker::registerObject()
{
    if (!m_initialized) {
        qDebug() << "Initializing dbus mock for org.freedesktop.ModemManager1.Modem...";

        new ModemAdaptor{this};

        auto bus = QDBusConnection::systemBus();
        bool status = bus.registerObject(dbusPath().path(), this);

        if (status) {
            m_initialized = true;
        } else {
            qDebug() << "Failed to init!";
        }
    }
}