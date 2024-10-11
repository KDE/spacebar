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

// Mock for org.freedesktop.ModemManager1.xml
class RootMocker : public QObject, public DBusObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ModemManager1")

public:
    RootMocker(QObject *parent = nullptr);

    void registerObject();

private:
    bool m_initialized{false};
};