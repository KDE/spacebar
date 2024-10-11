// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "server.h"

#include <QDBusConnection>
#include <QDateTime>

Server::Server(QObject *parent)
    : QObject{parent}
{
    init();
}

Server *Server::instance()
{
    static Server *server = new Server(nullptr);
    return server;
}

void Server::init()
{
    auto bus = QDBusConnection::systemBus();
    bool status = bus.registerService(QStringLiteral("org.freedesktop.ModemManager1"));

    if (!status) {
        qDebug() << "Failed to register service!";
        return;
    }

    m_objectManagerMocker = new ObjectManagerMocker(this);
    m_objectManagerMocker->registerObject();

    m_messagingMocker = new MessagingMocker(this, m_objectManagerMocker);
    m_messagingMocker->registerObject();
    m_objectManagerMocker->addObject(m_messagingMocker);

    m_modemMocker = new ModemMocker(this);
    m_modemMocker->registerObject();
    m_objectManagerMocker->addObject(m_modemMocker);

    m_rootMocker = new RootMocker(this);
    m_rootMocker->registerObject();
    m_objectManagerMocker->addObject(m_rootMocker);
}

void Server::sendMessage(const QString &message, const QString &number)
{
    if (!m_messagingMocker) {
        return;
    }

    m_messagingMocker->newMessage(message, number, true);
}

QString Server::messageLog() const
{
    return m_messageLog;
}

void Server::addMessageToLog(const QString &message)
{
    m_messageLog += message;
    m_messageLog += QStringLiteral("\n");
    Q_EMIT messageLogUpdated();
}
