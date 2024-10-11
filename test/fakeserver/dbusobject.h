/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QDBusObjectPath>
#include <QVariant>

class DBusObject
{
public:
    explicit DBusObject();
    virtual ~DBusObject() = default;

    QObject *objectParent() const;
    void setObjectParent(QObject *parent);

    QDBusObjectPath dbusPath() const;
    void setDBusPath(const QDBusObjectPath &path);

    QString dbusName() const;
    void setDBusName(const QString &name);

    bool haveDBusProperties() const;

    QVariantMap dbusProperties() const;
    void setDBusProperties(const QVariantMap &properties);

    QVariant dbusProperty(const QString &name) const;
    void changeDBusProperty(const QString &name, const QVariant &value);

private:
    QObject *m_parent;
    QDBusObjectPath m_path;
    QString m_name;
    QVariantMap m_properties;
};
