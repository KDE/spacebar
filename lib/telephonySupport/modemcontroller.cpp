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
    connect(ModemManager::notifier(), &ModemManager::Notifier::modemAdded, this, [this](const QString &udi) {
        init(udi);
    });
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
    if (modemPath) {
        m_modem = ModemManager::findModemDevice(*modemPath);
    } else {
        ModemManager::ModemDevice::List devices = ModemManager::modemDevices();
        if (!devices.isEmpty()) {
            m_modem = devices.first();
        }
    }

    if (!m_modem) {
        qWarning() << "Could not find modem" << modemPath.value_or(QString());
        return;
    }

    if (m_modem->hasInterface(ModemManager::ModemDevice::GsmInterface)) {
        m_modem3gpp = m_modem->interface(ModemManager::ModemDevice::GsmInterface).objectCast<ModemManager::Modem3gpp>();
        countryCode = m_modem3gpp->countryCode();
        qDebug() << "Country Code:" << countryCode;

        connect(m_modem3gpp.get(), &ModemManager::Modem3gpp::countryCodeChanged, this, [this](const QString &code) {
            countryCode = code;
            Q_EMIT countryCodeChanged(code);
            qDebug() << "country code changed" << code;
        });

        if (countryCode.isEmpty()) {
            qWarning() << "Country code is null! Phone numbers may not be interpreted correctly";
        }
    }

    m_interface = m_modem->modemInterface();

    connect(m_interface.get(), &ModemManager::Modem::bearerAdded, this, [this](const QString &bearer) {
        m_bearer = m_interface->findBearer(bearer);

        QList<QSharedPointer<ModemManager::Bearer>> bearers = m_interface->listBearers();
        for (const ModemManager::Bearer::Ptr &item : bearers) {
            if (item->uni() != bearer) {
                item->disconnect();
                m_interface->deleteBearer(item->uni());
            }
        }

        // whether or not the bearer is connected and thus whether packet data communication using this bearer is possible
        connect(m_bearer.data(), &ModemManager::Bearer::connectedChanged, this, [this](bool isConnected) {
            Q_EMIT modemDataConnectedChanged(isConnected);
            qDebug() << "bearer connected:" << isConnected;
        });

        // In some devices, packet data service will be suspended while the device is handling other communication, like a voice call
        connect(m_bearer.data(), &ModemManager::Bearer::suspendedChanged, this, [this](bool isSuspended) {
            Q_EMIT modemDataConnectedChanged(!isSuspended);
            qDebug() << "bearer suspended:" << isSuspended;
        });

        dnsServers = getDNS(m_bearer);
        qDebug() << "dns:" << dnsServers;
        connect(m_bearer.data(), &ModemManager::Bearer::ip4ConfigChanged, this, [this, bearers](const ModemManager::IpConfig &ipv4Config) {
            dnsServers = getDNS(m_bearer);
            qDebug() << "dns4 updated:" << dnsServers;
        });

        connect(m_bearer.data(), &ModemManager::Bearer::ip6ConfigChanged, this, [this, bearers](const ModemManager::IpConfig &ipv6Config) {
            dnsServers = getDNS(m_bearer);
            qDebug() << "dns6 updated:" << dnsServers;
        });

        ifaceName = m_bearer->interface();
        qDebug() << "interface:" << ifaceName;
        connect(m_bearer.data(), &ModemManager::Bearer::interfaceChanged, this, [this](const QString &iface) {
            ifaceName = iface;
            qDebug() << "interface changed:" << ifaceName;
        });
    });

    connect(m_modem.get(), &ModemManager::ModemDevice::interfaceAdded, this, [this](ModemManager::ModemDevice::InterfaceType type) {
        if (type == ModemManager::ModemDevice::MessagingInterface) {
            initMessaging();
        }
    });

    if (m_modem->hasInterface(ModemManager::ModemDevice::MessagingInterface)) {
        initMessaging();
    }

    connect(m_interface.get(), &ModemManager::Modem::stateChanged, this, [this](MMModemState oldState, MMModemState newState, MMModemStateChangeReason reason) {
        Q_UNUSED(oldState);
        Q_UNUSED(reason);
        if (newState == MMModemState::MM_MODEM_STATE_CONNECTED) {
            Q_EMIT modemConnected();
            Q_EMIT modemDataConnectedChanged(m_bearer ? m_bearer->isConnected() : false);
        }
    });
}

void ModemController::initMessaging()
{
    Q_ASSERT(m_modem);
    Q_ASSERT(m_modem->hasInterface(ModemManager::ModemDevice::MessagingInterface));

    m_msgManager = m_modem->messagingInterface();

    connect(m_msgManager.get(), &ModemManager::ModemMessaging::messageAdded, this, &ModemController::slotMessageAdded, Qt::UniqueConnection);

    Q_EMIT modemConnected();
}

void ModemController::slotMessageAdded(const QString &uni, bool received)
{
    // true if the message was received from the network, as opposed to being added locally
    if (!received) {
        return;
    }

    ModemManager::Sms::Ptr msg = m_msgManager->findMessage(uni);
    Q_ASSERT(msg);

    if (msg->state() == MMSmsState::MM_SMS_STATE_RECEIVING) {
        connect(msg.get(), &ModemManager::Sms::dataChanged, this, [this, msg]() {
            if (msg->state() == MMSmsState::MM_SMS_STATE_RECEIVED) {
                if (!msg->data().isEmpty()) {
                    Q_EMIT messageAdded(msg);
                }
            }
        });
        connect(msg.get(), &ModemManager::Sms::textChanged, this, [this, msg]() {
            if (msg->state() == MMSmsState::MM_SMS_STATE_RECEIVED) {
                if (!msg->text().isEmpty()) {
                    Q_EMIT messageAdded(msg);
                }
            }
        });
    } else {
        Q_EMIT messageAdded(msg);
    }
}

void ModemController::deleteMessage(const QString &uni)
{
    if (!m_msgManager) {
        return;
    }

    m_msgManager->deleteMessage(uni);
}

ModemManager::Sms::List ModemController::messages()
{
    if (!m_msgManager) {
        ModemManager::Sms::List list;
        return list;
    }

    return m_msgManager->messages();
}

QString ModemController::ownNumber()
{
    if (!m_interface) {
        return QString();
    }

    const QStringList numbers = m_interface->ownNumbers();

    if (!numbers.isEmpty()) {
        if (numbers.startsWith(QStringLiteral("+"))) {
            return numbers.first();
        }
        return QStringLiteral("+") + numbers.first();
    }

    return QString();
}

QString ModemController::getDNS(QSharedPointer<ModemManager::Bearer> bearer)
{
    QStringList dnsServerList = {bearer->ip4Config().dns1(),
                                 bearer->ip4Config().dns2(),
                                 bearer->ip4Config().dns3(),
                                 bearer->ip6Config().dns1(),
                                 bearer->ip6Config().dns2(),
                                 bearer->ip6Config().dns3()};

    // remove empty items
    dnsServerList.removeAll(QString());

    return dnsServerList.join(QStringLiteral(","));
}
