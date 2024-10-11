// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "objectmanagermocker.h"
#include "objectmanageradaptor.h"

#include <QDBusConnection>
#include <QDBusMetaType>

ObjectManagerMocker::ObjectManagerMocker(QObject *parent)
    : QObject{parent}
{
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<DBusManagerStruct>();
}

void ObjectManagerMocker::registerObject()
{
    if (!m_initialized) {
        qDebug() << "Initializing dbus mock for org.freedesktop.DBus.ObjectManager...";

        new ObjectManagerAdaptor{this};

        auto bus = QDBusConnection::systemBus();
        bool status = bus.registerObject(QStringLiteral("/org/freedesktop/ModemManager1"), this);

        if (status) {
            m_initialized = true;
        } else {
            qDebug() << "Failed to init!";
        }
    }
}

void ObjectManagerMocker::addObject(DBusObject *object)
{
    m_objects.insert(object->dbusPath().path(), object);

    QVariantMapMap interfaces;
    interfaces.insert(object->dbusName(), object->dbusProperties());
    Q_EMIT InterfacesAdded(object->dbusPath(), interfaces);
}

void ObjectManagerMocker::removeObject(DBusObject *object)
{
    m_objects.remove(object->dbusPath().path());
    Q_EMIT InterfacesRemoved(object->dbusPath(), QStringList(object->dbusName()));
}

DBusManagerStruct ObjectManagerMocker::GetManagedObjects()
{
    DBusManagerStruct objects;

    for (DBusObject *object : m_objects.values()) {
        if (objects.value(object->dbusPath()).isEmpty()) {
            objects[object->dbusPath()].insert(QStringLiteral("org.freedesktop.DBus.Introspectable"), QVariantMap());
            if (object->haveDBusProperties()) {
                objects[object->dbusPath()].insert(QStringLiteral("org.freedesktop.DBus.Properties"), QVariantMap());
            }
        }
        objects[object->dbusPath()].insert(object->dbusName(), object->dbusProperties());
    }

    return objects;
}
