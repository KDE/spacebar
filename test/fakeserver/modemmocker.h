// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <QDBusObjectPath>
#include <QList>
#include <QVariantMap>

#include <ModemManagerQt/Manager>

#include "dbusobject.h"

typedef QList<uint> UIntList;

// Mock for org.freedesktop.ModemManager1.Modem.xml
class ModemMocker : public QObject, public DBusObject
{
    Q_OBJECT
    Q_PROPERTY(QDBusObjectPath Sim READ sim NOTIFY simChanged)
    Q_PROPERTY(QList<QDBusObjectPath> Bearers READ bearers NOTIFY bearersChanged)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ModemManager1.Modem")

public:
    ModemMocker(QObject *parent = nullptr);

    QDBusObjectPath sim() const
    {
        return {};
    }
    QList<QDBusObjectPath> bearers() const
    {
        return {};
    }

    void registerObject();

Q_SIGNALS:
    void simChanged();
    void bearersChanged();

private:
    bool m_initialized{false};
};