// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QObject>
#include <QDBusPendingReply>

#include <optional>

#include <ModemManagerQt/ModemMessaging>
#include <ModemManagerQt/Sms>

class ModemController : public QObject
{
    Q_OBJECT
public:
    static ModemController &instance();

    void init(std::optional<QString> modemPath);

    std::optional<QDBusPendingReply<QDBusObjectPath>> createMessage(ModemManager::ModemMessaging::Message m);

Q_SIGNALS:
    void messageAdded(ModemManager::Sms::Ptr message, bool received);

private:
    ModemController();

    ModemManager::ModemMessaging::Ptr m_msgManager;
};
