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

    connect(m_modem.get(), &ModemManager::ModemDevice::interfaceAdded, this, [this](ModemManager::ModemDevice::InterfaceType type) {
        if (type == ModemManager::ModemDevice::MessagingInterface) {
            initMessaging();
        }
    });

    if (m_modem->hasInterface(ModemManager::ModemDevice::MessagingInterface)) {
        initMessaging();
    }

    m_interface = m_modem->modemInterface();

    QList<QSharedPointer<ModemManager::Bearer> > bearers = m_interface->listBearers();

    if (bearers.isEmpty()) {
        return;
    }

    connect(m_interface.get(), &ModemManager::Modem::stateChanged, this, [this, bearers](MMModemState oldState, MMModemState newState, MMModemStateChangeReason reason) {
        Q_UNUSED(oldState);
        Q_UNUSED(reason);
        if (newState == MMModemState::MM_MODEM_STATE_CONNECTED) {
            Q_EMIT modemConnected();
            Q_EMIT modemDataConnectedChanged(bearers.first()->isConnected());
        }
    });

    connect(bearers.first().data(), &ModemManager::Bearer::connectedChanged, this, [this](bool isConnected) {
        Q_EMIT modemDataConnectedChanged(isConnected);
    });
}

void ModemController::initMessaging()
{
    Q_ASSERT(m_modem);
    Q_ASSERT(m_modem->hasInterface(ModemManager::ModemDevice::MessagingInterface));

    m_msgManager = m_modem->messagingInterface();

    connect(m_msgManager.get(), &ModemManager::ModemMessaging::messageAdded, this, &ModemController::slotMessageAdded, Qt::UniqueConnection);
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
        return numbers.first();
    }

    return QString();
}
