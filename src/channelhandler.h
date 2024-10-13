// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include "daemoninterface.h"

#include <database.h>

#include <optional>

class ChannelHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDaemonAvailable READ isInterfaceAvailable NOTIFY isInterfaceAvailableChanged)

public:
    explicit ChannelHandler(QObject *parent = nullptr);

    Database &database();
    org::kde::spacebar::Daemon *interface();

    bool isInterfaceAvailable();

private:
    void connectInterface();
    void connectInterfaceSignals();

    Database m_database;
    org::kde::spacebar::Daemon *m_interface{nullptr};
    bool m_isInterfaceAvailable{false};
    QDBusServiceWatcher *m_watcher{nullptr};

Q_SIGNALS:
    void isInterfaceAvailableChanged();
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
    void messagesChanged(const PhoneNumberList &phoneNumberList);
};
