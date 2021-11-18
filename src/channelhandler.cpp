// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channelhandler.h"

#include "utils.h"
#include "databasethread.h"
#include "modemcontroller.h"

#include <global.h>
#include <database.h>

ChannelHandler::ChannelHandler(std::optional<QString> &modemPath, QObject *parent)
    : QObject(parent)
{
    ModemController::instance().init(modemPath);

    // daemon dbus interface
    m_interface = new org::kde::spacebar::Daemon(QStringLiteral("org.kde.Spacebar"), QStringLiteral("/Daemon"), QDBusConnection::sessionBus(), this);

    // Refresh chat list when message arrives
    // The message will be saved by the background daemon
    connect(m_interface, &OrgKdeSpacebarDaemonInterface::messageAdded, [this](const QString &phoneNumber, const QString &id) {
        Q_UNUSED(id);
        Q_EMIT m_databaseThread.database().messagesChanged(PhoneNumberList(phoneNumber));
    });
}

AsyncDatabase &ChannelHandler::database()
{
    return m_databaseThread.database();
}

org::kde::spacebar::Daemon *ChannelHandler::interface()
{
    return m_interface;
}

Mms &ChannelHandler::mms()
{
    return m_mms;
}
