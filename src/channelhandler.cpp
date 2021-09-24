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

#include <ModemManagerQt/Sms>

ChannelHandler::ChannelHandler(std::optional<QString> &modemPath, QObject *parent)
    : QObject(parent)
{
    ModemController::instance().init(modemPath);

    // Refresh chat list when message arrives
    // The message will be saved by the background daemon
    connect(&ModemController::instance(), &ModemController::messageAdded, this, [=, this](ModemManager::Sms::Ptr msg, bool received) {
        Q_UNUSED(received);
        Q_EMIT m_databaseThread.database().messagesChanged(msg->number());
    });
}

AsyncDatabase &ChannelHandler::database()
{
    return m_databaseThread.database();
}
