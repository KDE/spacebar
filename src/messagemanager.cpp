// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "messagemanager.h"

#include <QFutureInterface>

#include <global.h>

MessageManager::MessageManager(QObject *parent)
    : QOfonoMessageManager(parent)
{
}

QFuture<std::pair<bool, QString>> MessageManager::sendMessage(const QString &to, const QString &text)
{
    const auto futureInterface = std::make_shared<QFutureInterface<std::pair<bool, QString>>>();

    auto *iface = dbusInterface();
    if (iface) {
        connect(new QDBusPendingCallWatcher(iface->asyncCall(SL("SendMessage"), to, text), iface),
            &QDBusPendingCallWatcher::finished,
            [=](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                QDBusPendingReply<QDBusObjectPath> reply = *watcher;
                if (reply.isError()) {
                    qWarning() << reply.error();
                    futureInterface->reportResult(std::make_pair(false, QString()));
                    futureInterface->reportFinished();
                } else {
                    futureInterface->reportResult(std::make_pair(true, reply.value().path()));
                    futureInterface->reportFinished();
                }
        });
    } else {
        futureInterface->reportResult(std::make_pair(false, QString()));
        futureInterface->reportFinished();
    }

    return futureInterface->future();
}
