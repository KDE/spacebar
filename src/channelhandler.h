// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <optional>

#include "daemoninterface.h"

class ChannelHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDaemonAvailable READ isInterfaceAvailable NOTIFY isInterfaceAvailableChanged)

public:
    explicit ChannelHandler(QObject *parent = nullptr);

    org::kde::spacebar::Daemon *interface();

    bool isInterfaceAvailable();

private:
    void connectInterface();
    void connectInterfaceSignals();

    org::kde::spacebar::Daemon *m_interface{nullptr};
    bool m_isInterfaceAvailable{false};
    QDBusServiceWatcher *m_watcher{nullptr};

Q_SIGNALS:
    void isInterfaceAvailableChanged();
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
    void messagesChanged(const PhoneNumberList &phoneNumberList);
};
