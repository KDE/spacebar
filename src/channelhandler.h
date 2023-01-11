// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <databasethread.h>

#include "daemoninterface.h"

#include <optional>

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChannelHandler(QObject *parent = nullptr);

    AsyncDatabase &database();
    org::kde::spacebar::Daemon *interface();

private:
    DatabaseThread m_databaseThread;
    org::kde::spacebar::Daemon *m_interface;

Q_SIGNALS:
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
    void messagesChanged(const PhoneNumberList &phoneNumberList);
};
