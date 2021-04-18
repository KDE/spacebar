// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "sofonomessagemanager.h"

#include <QFutureInterface>

#include <global.h>

SOfonoMessageManager::SOfonoMessageManager(QObject *parent)
    : QOfonoMessageManager(parent)
{
}

QFuture<std::tuple<bool, QString>> SOfonoMessageManager::sendMessage(const QString &to, const QString &text)
{
    const auto futureInterface = std::make_shared<QFutureInterface<std::tuple<bool, QString>>>();

    auto *iface = dbusInterface();
    if (iface) {
        connect(new QDBusPendingCallWatcher(iface->asyncCall(SL("SendMessage"), to, text), iface),
            &QDBusPendingCallWatcher::finished,
            [=](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                QDBusPendingReply<QDBusObjectPath> reply = *watcher;
                if (reply.isError()) {
                    qWarning() << reply.error();
                    futureInterface->reportResult(std::make_tuple(false, QString()));
                    futureInterface->reportFinished();
                } else {
                    futureInterface->reportResult(std::make_tuple(true, reply.value().path()));
                    futureInterface->reportFinished();
                }
        });
    } else {
        futureInterface->reportResult(std::make_tuple(false, QString()));
        futureInterface->reportFinished();
    }

    return futureInterface->future();
}
