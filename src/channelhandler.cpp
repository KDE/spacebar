// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channelhandler.h"

#include <QLocale>

#include <global.h>

const QString DAEMON_DBUS_SERVICE = QStringLiteral("org.kde.spacebar.Daemon");

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent)
{
    // Daemon DBus interface
    connectInterface();
    m_isInterfaceAvailable = m_interface->isValid();

    qDebug() << "Is Daemon DBus service available:" << m_isInterfaceAvailable;
    if (m_isInterfaceAvailable) {
        connectInterfaceSignals();
    }

    // Watch if the spacebar daemon goes missing
    m_watcher = new QDBusServiceWatcher(DAEMON_DBUS_SERVICE, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [this]() -> void {
        connectInterface();

        m_isInterfaceAvailable = m_interface->isValid();
        Q_EMIT isInterfaceAvailableChanged();
        qDebug() << "Daemon DBus service registered, is service available:" << m_isInterfaceAvailable;

        if (m_isInterfaceAvailable) {
            connectInterfaceSignals();
        }
    });
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this]() -> void {
        m_isInterfaceAvailable = false;
        Q_EMIT isInterfaceAvailableChanged();
        qDebug() << "Daemon DBus service unregistered";
    });

    const QLocale locale;
    const QStringList qcountry = locale.name().split(u'_');
    QString countryCode = qcountry.constLast();
    PhoneNumber::setCountryCode(countryCode);

    // prefer locale country code, but use modem country code if not set
    if (countryCode == QStringLiteral("C")) {
        countryCode = m_interface->countryCode();
        if (countryCode != SL("The name org.kde.Spacebar was not provided by any .service files")) {
            PhoneNumber::setCountryCode(countryCode);
        }
    }
}

void ChannelHandler::connectInterface()
{
    if (m_interface) {
        m_interface->deleteLater();
    }
    m_interface = new org::kde::spacebar::Daemon(DAEMON_DBUS_SERVICE, QStringLiteral("/Daemon"), QDBusConnection::sessionBus(), this);
}

void ChannelHandler::connectInterfaceSignals()
{
    // Update the chat list when message arrives
    // The message is saved to the database by the background daemon
    connect(m_interface, &OrgKdeSpacebarDaemonInterface::messageAdded, [this](const QString &phoneNumber, const QString &id) {
        Q_UNUSED(id);
        Q_EMIT messagesChanged(PhoneNumberList(phoneNumber));
    });
}

org::kde::spacebar::Daemon *ChannelHandler::interface()
{
    return m_interface;
}

bool ChannelHandler::isInterfaceAvailable()
{
    return m_isInterfaceAvailable;
}