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

class ChannelHandler : public QObject, public Tp::AbstractClientHandler
{
    Q_OBJECT

public:
    explicit ChannelHandler(QObject *parent = nullptr);

    bool bypassApproval() const override { return true; };
    void handleChannels(const Tp::MethodInvocationContextPtr<> &context, const Tp::AccountPtr &, const Tp::ConnectionPtr &,
        const QList<Tp::ChannelPtr> &channels, const QList<Tp::ChannelRequestPtr> &, const QDateTime &, const Tp::AbstractClientHandler::HandlerInfo &) override;

    /*
     * Finds a way to get a channel for the phone number. Either it finds one in the channels that are already open
     * or creates an new one
     * emits channelOpen when it finished
     */
    void openChannel(const QString &phoneNumber);

    Database *database() const;

private:

    QVector<Tp::TextChannelPtr> m_channels;
    Tp::AccountPtr m_simAccount;
    Database *m_database;

signals:
    void handlerReady();
    void channelOpen(Tp::TextChannelPtr pc, const QString &phoneNumber);
};
