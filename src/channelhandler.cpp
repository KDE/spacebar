// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channelhandler.h"

#include <KLocalizedString>

#include "utils.h"
#include "databasethread.h"
#include <global.h>
#include <database.h>

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent)
{
    connect(&m_manager, &QOfonoManager::defaultModemChanged, this, [&] {
        m_msgManager.setModemPath(m_manager.defaultModem());
    });

    m_msgManager.setModemPath(m_manager.defaultModem());

    // Refresh chat list when message arrives
    // The message will be saved by the background daemon
    connect(&m_msgManager, &QOfonoMessageManager::incomingMessage, this, [&](const QString &, const QVariantMap &info) {
        Q_EMIT m_databaseThread.database().messagesChanged(info[SL("Sender")].toString());
    });
}

AsyncDatabase &ChannelHandler::database()
{
    return m_databaseThread.database();
}

MessageManager &ChannelHandler::msgManager()
{
    return m_msgManager;
}
