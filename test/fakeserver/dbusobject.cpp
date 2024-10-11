/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "dbusobject.h"

#include <QDBusConnection>
#include <QDBusMessage>

DBusObject::DBusObject()
    : m_parent(nullptr)
{
}

QObject *DBusObject::objectParent() const
{
    return m_parent;
}

void DBusObject::setObjectParent(QObject *parent)
{
    m_parent = parent;
}

QDBusObjectPath DBusObject::dbusPath() const
{
    return m_path;
}

void DBusObject::setDBusPath(const QDBusObjectPath &path)
{
    m_path = path;
}

QString DBusObject::dbusName() const
{
    return m_name;
}

void DBusObject::setDBusName(const QString &name)
{
    m_name = name;
}

bool DBusObject::haveDBusProperties() const
{
    return !m_properties.isEmpty();
}

QVariantMap DBusObject::dbusProperties() const
{
    return m_properties;
}

void DBusObject::setDBusProperties(const QVariantMap &properties)
{
    m_properties = properties;
}

QVariant DBusObject::dbusProperty(const QString &name) const
{
    return m_properties.value(name);
}

void DBusObject::changeDBusProperty(const QString &name, const QVariant &value)
{
    if (m_properties.value(name) == value) {
        return;
    }

    QVariantMap updatedProperties;
    updatedProperties[name] = value;

    m_properties[name] = value;

    QDBusMessage signal = QDBusMessage::createSignal(m_path.path(), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    signal << m_name;
    signal << updatedProperties;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}