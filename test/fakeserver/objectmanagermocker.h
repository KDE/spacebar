// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <QDBusObjectPath>
#include <QList>
#include <QMultiMap>
#include <QVariantMap>

#include <ModemManagerQt/Manager>

#include "dbusobject.h"

typedef QMap<QString, QVariantMap> QVariantMapMap;

typedef QMap<QDBusObjectPath, QVariantMapMap> DBusManagerStruct;

typedef QList<uint> UIntList;

// Mock for org.freedesktop.ModemManager1.Modem.xml
class ObjectManagerMocker : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")

public:
    ObjectManagerMocker(QObject *parent = nullptr);

    void registerObject();

    void addObject(DBusObject *object);
    void removeObject(DBusObject *object);

public Q_SLOTS:
    Q_SCRIPTABLE DBusManagerStruct GetManagedObjects();

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &object, const QVariantMapMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &object, const QStringList &interfaces);

private:
    bool m_initialized{false};

    QMultiMap<QString, DBusObject *> m_objects;
};