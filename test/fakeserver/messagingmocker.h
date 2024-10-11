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
#include "objectmanagermocker.h"
#include "smsmocker.h"

typedef QList<uint> UIntList;

// Mock for org.freedesktop.ModemManager1.Modem.Messaging.xml
class MessagingMocker : public QObject, public DBusObject
{
    Q_OBJECT
    Q_PROPERTY(QList<uint> SupportedStorages READ supportedStorages)
    Q_PROPERTY(uint DefaultStorage READ defaultStorage)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ModemManager1.Modem.Messaging")

public:
    MessagingMocker(QObject *parent = nullptr, ObjectManagerMocker *objectManager = nullptr);

    void registerObject();
    QDBusObjectPath newMessage(const QString &message, const QString &number, bool isReceiving);

    UIntList supportedStorages() const;
    uint defaultStorage() const;

public Q_SLOTS:
    Q_SCRIPTABLE QList<QDBusObjectPath> List();
    Q_SCRIPTABLE void Delete(QDBusObjectPath in);
    Q_SCRIPTABLE QDBusObjectPath Create(QVariantMap properties);

Q_SIGNALS:
    Q_SCRIPTABLE void Added(QDBusObjectPath path, bool received);
    Q_SCRIPTABLE void Deleted(QDBusObjectPath path);

private:
    bool m_initialized{false};

    ObjectManagerMocker *m_objectManager{nullptr};

    QList<SmsMocker *> m_smsList;
    int m_idCounter{0};
};