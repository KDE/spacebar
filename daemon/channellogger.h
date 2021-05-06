// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/Channel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>
#include <TelepathyQt/Types>

#include "database.h"

namespace Tp {
class PendingChannel;
class ReceivedMessage;
}

class ChannelLogger : public QObject, public Tp::AbstractClientObserver
{
    Q_OBJECT

public:
    explicit ChannelLogger(QObject *parent = nullptr);


    void observeChannels(const Tp::MethodInvocationContextPtr<> &context,
            const Tp::AccountPtr &account,
            const Tp::ConnectionPtr &connection,
            const QList<Tp::ChannelPtr> &channels,
            const Tp::ChannelDispatchOperationPtr &dispatchOperation,
            const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
            const ObserverInfo &observerInfo) override;

private:
    void handleIncomingMessage(const Tp::TextChannelPtr&, const Tp::ReceivedMessage &receivedMessage);
    void handleOutgoingMessage(Tp::TextChannelPtr channel, const Tp::Message &sentMessage);

    QVector<Tp::TextChannelPtr> m_channels;
    Tp::AccountPtr m_simAccount;
    Database *m_database;

signals:
    void handlerReady();
};
