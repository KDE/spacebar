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

// Mock for org.freedesktop.ModemManager1.Sms.xml
class SmsMocker : public QObject, public DBusObject
{
    Q_OBJECT
    Q_PROPERTY(QString Number READ number)
    Q_PROPERTY(QString Text READ text)
    Q_PROPERTY(QString Timestamp READ timestamp)
    Q_PROPERTY(QByteArray Data READ data)
    Q_PROPERTY(int State READ state)
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ModemManager1.Sms")

public:
    SmsMocker(QObject *parent = nullptr, QString number = {}, QString text = {}, QString timestamp = {}, QString id = {}, int state = 0);

    void registerObject();

    QString number() const;
    QString text() const;
    QString timestamp() const;
    QByteArray data() const
    {
        return {};
    }
    int state() const;

    Q_SCRIPTABLE void Send()
    {
    }
    Q_SCRIPTABLE void Store(uint storage)
    {
        Q_UNUSED(storage)
    }

private:
    bool m_initialized{false};
    QString m_number;
    QString m_text;
    QString m_timestamp;
    int m_state;

    QDBusObjectPath m_path;
};