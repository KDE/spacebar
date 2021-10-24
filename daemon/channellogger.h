// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <optional>

#include <database.h>
#include "modemcontroller.h"

class ChannelLogger : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.spacebar.Daemon")

public:
    explicit ChannelLogger(std::optional<QString> &modemPath, QObject *parent = nullptr);

    Q_SCRIPTABLE void disableNotificationsForNumber(const QString &phoneNumber);

private:
    void handleIncomingMessage(ModemManager::Sms::Ptr msg);

    Database m_database;

    QString m_disabledNotificationNumber;
};
