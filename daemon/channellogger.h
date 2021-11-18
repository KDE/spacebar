// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <optional>

#include <database.h>
#include <mms.h>
#include "modemcontroller.h"

class ChannelLogger : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.spacebar.Daemon")

public:
    explicit ChannelLogger(std::optional<QString> &modemPath, QObject *parent = nullptr);

    // QString argument since this needs to be called from DBus
    Q_SCRIPTABLE void disableNotificationsForNumber(const QString &numbers);
    Q_SCRIPTABLE void manualDownload(const QString &id, const QString &url, const QDateTime &expires);
    Q_SCRIPTABLE void syncSettings();

private:
    void checkMessages();
    void handleIncomingMessage(ModemManager::Sms::Ptr msg);
    void createDownloadNotification(const MmsMessage &mmsMessage);
    void saveMessage(
        const PhoneNumberList &phoneNumberList,
        const QDateTime &datetime,
        const QString &text = QString(),
        const QString &attachments = QString(),
        const QString &smil = QString(),
        const QString &fromNumber = QString(),
        const QString &messageId = QString(),
        const bool pendingDownload = false,
        const QString &contentLocation = QString(),
        const QDateTime &expires = QDateTime(),
        const int size = 0
    );

    Database m_database;

    Mms m_mms;

    PhoneNumberList m_disabledNotificationNumber;

    PhoneNumber m_ownNumber;

    bool m_dataConnected;

    QStringList m_deferredIndicators;

private slots:
    void handleDownloadedMessage(const QByteArray &response, const QString &url, const QDateTime &expires);

signals:
    Q_SCRIPTABLE void messageAdded(const QString &phoneNumber, const QString &id);
    Q_SCRIPTABLE void manualDownloadFinished(const QString &id, const bool isEmpty);
};
