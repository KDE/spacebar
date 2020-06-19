// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/Channel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Account>

#include "database.h"

namespace Tp {
class PendingChannel;
class ReceivedMessage;
}

class ChannelLogger : public QObject, public Tp::AbstractClientHandler
{
    Q_OBJECT

public:
    explicit ChannelLogger(QObject *parent = nullptr);

    bool bypassApproval() const override { return true; };
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context, const Tp::AccountPtr &, const Tp::ConnectionPtr &,
        const QList<Tp::ChannelPtr> &channels, const QList<Tp::ChannelRequestPtr> &, const QDateTime &, const Tp::AbstractClientHandler::HandlerInfo &) override;

private:
    void handleIncomingMessage(Tp::TextChannelPtr, const Tp::ReceivedMessage &receivedMessage);
    void handleOutgoingMessage(Tp::TextChannelPtr channel, const Tp::Message &sentMessage);

    QVector<Tp::TextChannelPtr> m_channels;
    Tp::AccountPtr m_simAccount;
    Database *m_database;

signals:
    void handlerReady();
};
