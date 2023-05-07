// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channelhandler.h"

#include <QLocale>

#include <database.h>
#include <global.h>

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent)
{
    // daemon dbus interface
    m_interface = new org::kde::spacebar::Daemon(QStringLiteral("org.kde.Spacebar"), QStringLiteral("/Daemon"), QDBusConnection::sessionBus(), this);

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

    // Update the chat list when message arrives
    // The message is saved to the database by the background daemon
    connect(m_interface, &OrgKdeSpacebarDaemonInterface::messageAdded, [this](const QString &phoneNumber, const QString &id) {
        Q_UNUSED(id);
        Q_EMIT messagesChanged(PhoneNumberList(phoneNumber));
    });
}

Database &ChannelHandler::database()
{
    return m_database;
}

org::kde::spacebar::Daemon *ChannelHandler::interface()
{
    return m_interface;
}
