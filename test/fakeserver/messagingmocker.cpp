// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagingmocker.h"
#include "messagingadaptor.h"
#include "server.h"

#include <QDBusConnection>

#include <iostream>

MessagingMocker::MessagingMocker(QObject *parent, ObjectManagerMocker *objectManager)
    : QObject{parent}
    , m_objectManager{objectManager}
{
    setDBusName(QStringLiteral("org.freedesktop.ModemManager1.Modem.Messaging"));
    setDBusPath(QDBusObjectPath(QStringLiteral("/org/freedesktop/ModemManager1/Modem/0")));
}

void MessagingMocker::registerObject()
{
    if (!m_initialized) {
        qDebug() << "Initializing dbus mock for org.freedesktop.ModemManager1.Modem.Messaging...";

        new MessagingAdaptor{this};

        auto bus = QDBusConnection::systemBus();
        bool status = bus.registerObject(dbusPath().path(), this);

        if (status) {
            m_initialized = true;
        } else {
            qDebug() << "Failed to init!";
        }
    }
}

QDBusObjectPath MessagingMocker::newMessage(const QString &message, const QString &number, bool isReceiving)
{
    if (!m_objectManager) {
        return {};
    }

    SmsMocker *mocker = new SmsMocker(this,
                                      number,
                                      message,
                                      QDateTime::currentDateTime().toString(Qt::ISODate),
                                      QString::number(m_idCounter),
                                      isReceiving ? MM_SMS_STATE_RECEIVED : MM_SMS_STATE_SENT);

    mocker->registerObject();
    m_objectManager->addObject(mocker);
    m_smsList.push_back(mocker);

    ++m_idCounter;

    Q_EMIT Added(mocker->dbusPath(), true);
    return mocker->dbusPath();
}

UIntList MessagingMocker::supportedStorages() const
{
    return {0};
}

uint MessagingMocker::defaultStorage() const
{
    return 0;
}

QList<QDBusObjectPath> MessagingMocker::List()
{
    qDebug() << "List called";
    Server::instance()->addMessageToLog(QStringLiteral("client requested message list"));

    QList<QDBusObjectPath> paths;
    for (const auto *mocker : m_smsList) {
        paths.push_back(mocker->dbusPath());
    }

    return paths;
}

void MessagingMocker::Delete(QDBusObjectPath in)
{
    qDebug() << "Delete called";
    Server::instance()->addMessageToLog(QStringLiteral("delete message from modem request: ") + in.path());

    for (int i = 0; i < m_smsList.size(); ++i) {
        if (m_smsList[i]->dbusPath() == in) {
            auto sms = m_smsList[i];
            m_smsList.removeAt(i);
            sms->deleteLater();
            --i;

            m_objectManager->removeObject(sms);
        }
    }
}

QDBusObjectPath MessagingMocker::Create(QVariantMap properties)
{
    if (!m_objectManager) {
        return {};
    }

    QString propertiesStr;
    for (QVariantMap::const_iterator iter = properties.begin(); iter != properties.end(); ++iter) {
        propertiesStr += iter.key() + QStringLiteral(": ") + iter.value().toString() + QStringLiteral(", ");
    }

    qDebug() << "Create called:" << properties;
    Server::instance()->addMessageToLog(QStringLiteral("new message request: ") + propertiesStr);

    QString data = properties[QStringLiteral("data")].toString();
    QString number = properties[QStringLiteral("number")].toString();
    QString text = properties[QStringLiteral("text")].toString();

    // "data" and "text" fields both could have the message, as per ModemManager API
    // ("data" is for chunking if the message gets too long)
    QString message = data;
    if (data.isEmpty()) {
        message = text;
    }

    QDBusObjectPath path = newMessage(message, number, false);
    qDebug() << "created message" << path.path();
    return path;
}
