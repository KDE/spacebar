// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <databasethread.h>

#include <mms.h>

#include "daemoninterface.h"

#include <optional>

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChannelHandler(std::optional<QString> &modemPath, QObject *parent = nullptr);

    AsyncDatabase &database();
    org::kde::spacebar::Daemon *interface();
    Mms &mms();

private:
    DatabaseThread m_databaseThread;
    org::kde::spacebar::Daemon *m_interface;
    Mms m_mms;

Q_SIGNALS:
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
};
