// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <databasethread.h>

#include <qofonomanager.h>

#include <optional>

#include "messagemanager.h"

class AsyncDatabase;

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChannelHandler(std::optional<QString> &modemPath, QObject *parent = nullptr);

    AsyncDatabase &database();
    MessageManager &msgManager();

private:
    DatabaseThread m_databaseThread;
    MessageManager m_msgManager;
    QOfonoManager m_manager;

signals:
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
};
