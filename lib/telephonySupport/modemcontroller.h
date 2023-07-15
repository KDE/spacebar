// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#pragma once

#include <QDBusPendingReply>
#include <QObject>

#include <optional>

#include <ModemManagerQt/Modem3Gpp>
#include <ModemManagerQt/ModemDevice>
#include <ModemManagerQt/ModemMessaging>
#include <ModemManagerQt/Sms>

class ModemController : public QObject
{
    Q_OBJECT
public:
    static ModemController &instance();

    void init(std::optional<QString> modemPath);

    std::optional<QDBusPendingReply<QDBusObjectPath>> createMessage(ModemManager::ModemMessaging::Message m);
    void deleteMessage(const QString &uni);

    ModemManager::Sms::List messages();

    QString ownNumber();

    QString ifaceName;

    QString dnsServers;

    QString countryCode;

Q_SIGNALS:
    void messageAdded(ModemManager::Sms::Ptr message);
    void modemConnected();
    void modemDataConnectedChanged(const bool isConnected);
    void countryCodeChanged(const QString &code);

private Q_SLOTS:
    void slotMessageAdded(const QString &uni, bool received);

private:
    void initMessaging();
    ModemController();

    ModemManager::ModemDevice::Ptr m_modem;
    ModemManager::ModemMessaging::Ptr m_msgManager;
    ModemManager::Modem::Ptr m_interface;
    ModemManager::Bearer::Ptr m_bearer;
    ModemManager::Modem3gpp::Ptr m_modem3gpp;

    QString getDNS(QSharedPointer<ModemManager::Bearer> bearer);
};
