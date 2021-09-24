// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <databasethread.h>

#include <optional>

class AsyncDatabase;

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChannelHandler(std::optional<QString> &modemPath, QObject *parent = nullptr);

    AsyncDatabase &database();

private:
    DatabaseThread m_databaseThread;

signals:
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
};
