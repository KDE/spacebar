// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <databasethread.h>

#include <qofonomanager.h>
#include <qofonomessagemanager.h>

class AsyncDatabase;

class ChannelHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChannelHandler(QObject *parent = nullptr);

    /*
     * Finds a way to get a channel for the phone number. Either it finds one in the channels that are already open
     * or creates an new one
     * emits channelOpen when it finished
     */
    void openChannel(const QString &phoneNumber);

    AsyncDatabase &database();
    QOfonoMessageManager &msgManager();

private:
    DatabaseThread m_databaseThread;
    QOfonoMessageManager m_msgManager;
    QOfonoManager m_manager;

signals:
    void handlerReady();
    void channelOpen(const QString &phoneNumber);
};
