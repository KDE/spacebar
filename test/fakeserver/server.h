// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include "messagingmocker.h"
#include "modemmocker.h"
#include "objectmanagermocker.h"
#include "rootmocker.h"
#include "smsmocker.h"

class Server : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString messageLog READ messageLog NOTIFY messageLogUpdated)

public:
    Server(QObject *parent = nullptr);
    void init();

    static Server *instance();

    QString messageLog() const;
    void addMessageToLog(const QString &message);

public Q_SLOTS:
    void sendMessage(const QString &message, const QString &number);

Q_SIGNALS:
    void messageLogUpdated();

private:
    ObjectManagerMocker *m_objectManagerMocker{nullptr};
    MessagingMocker *m_messagingMocker{nullptr};
    ModemMocker *m_modemMocker{nullptr};
    RootMocker *m_rootMocker{nullptr};

    QString m_messageLog;
};