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

QFuture<SendMessageResult> MessageManager::sendMessage(const QString &to, const QString &text)
{
    const auto futureInterface = std::make_shared<QFutureInterface<SendMessageResult>>();

    auto *iface = dbusInterface();
    if (!iface) {
        futureInterface->reportResult(ModemNotFoundError());
        futureInterface->reportFinished();

        return futureInterface->future();
    }

    connect(new QDBusPendingCallWatcher(iface->asyncCall(SL("SendMessage"), to, text), iface),
        &QDBusPendingCallWatcher::finished,
        [=](QDBusPendingCallWatcher *watcher) {
            watcher->deleteLater();

            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            if (reply.isError()) {
                futureInterface->reportResult(reply.error());
                futureInterface->reportFinished();
            } else {
                futureInterface->reportResult(reply.value());
                futureInterface->reportFinished();
            }
    });

    return futureInterface->future();
}
