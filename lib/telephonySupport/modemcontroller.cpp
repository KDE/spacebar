// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "modemcontroller.h"

#include <ModemManagerQt/Manager>

ModemController &ModemController::instance()
{
    static ModemController instance;
    return instance;
}

ModemController::ModemController()
    : QObject()
{
}

std::optional<QDBusPendingReply<QDBusObjectPath>> ModemController::createMessage(ModemManager::ModemMessaging::Message m)
{
    if (!m_msgManager) {
        return {};
    }

    return m_msgManager->createMessage(m);
}

void ModemController::init(std::optional<QString> modemPath)
{
    ModemManager::ModemDevice::Ptr modem = nullptr;

    if (modemPath) {
        modem = ModemManager::findModemDevice(*modemPath);
    } else {
        ModemManager::ModemDevice::List devices = ModemManager::modemDevices();
        if (!devices.isEmpty()) {
            modem = devices.first();
        }
    }

    if (!modem) {
        qWarning() << "Could not find modem" << modemPath.value_or(QString());
        return;
    }

    m_msgManager = modem->messagingInterface();

    connect(m_msgManager.get(), &ModemManager::ModemMessaging::messageAdded, this, [this](const QString &uni, bool received) {
        ModemManager::Sms::Ptr msg = m_msgManager->findMessage(uni);
        Q_ASSERT(msg);

        Q_EMIT messageAdded(msg, received);
    });
}
