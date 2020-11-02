// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "channelhandler.h"

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ReceivedMessage>

#include "utils.h"
#include "global.h"
#include "database.h"

ChannelHandler::ChannelHandler(QObject *parent)
    : QObject(parent)
    , Tp::AbstractClientHandler(Tp::ChannelClassSpecList({
          Tp::ChannelClassSpec::textChat(), Tp::ChannelClassSpec::unnamedTextChat()
      }))
    , m_database(new Database(this))
{
    // Set up sms account
    Tp::AccountManagerPtr manager = Tp::AccountManager::create();
    Tp::PendingReady *ready = manager->becomeReady();
    QObject::connect(ready, &Tp::PendingReady::finished, this, [=] {
        const Tp::AccountSetPtr accountSet = manager->validAccounts();
        const auto accounts = accountSet->accounts();
        for (const Tp::AccountPtr &account : accounts) {
            qDebug() << account->protocolName();

            static const QStringList supportedProtocols = {
                QLatin1String("ofono"),
                QLatin1String("tel"),
            };
            if (supportedProtocols.contains(account->protocolName())) {
                m_simAccount = account;
                break;
            }
        }

        if (m_simAccount.isNull()) {
            qCritical() << "Unable to get SIM account;"
                        << "is the telepathy-ofono or telepathy-ring backend installed?";
        }

        emit handlerReady();
    });
}

void ChannelHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
    const Tp::AccountPtr &/*account*/,
    const Tp::ConnectionPtr &/*connection*/,
    const QList<Tp::ChannelPtr> &channels,
    const QList<Tp::ChannelRequestPtr> &/*requestsSatisfied*/,
    const QDateTime &/*userActionTime*/,
    const Tp::AbstractClientHandler::HandlerInfo &/*handlerInfo*/)
{
    qDebug() << "handler called";
    for (const Tp::ChannelPtr &channel : channels) {
        auto textChannel = Tp::TextChannelPtr::qObjectCast(channel);
        if (!textChannel) {
            qDebug() << "channel is not a text channel. None of my business";
            continue;
        }

        const auto messageQueue = textChannel->messageQueue();
        qDebug() << "Found a new text channel, yay" << channel.data();
        if (!m_channels.contains(textChannel)) {
            m_channels.append(textChannel);
        }
    }

    context->setFinished();
}

void ChannelHandler::openChannel(const QString &phoneNumber)
{
    if (!m_simAccount) {
        Utils::instance()->showPassiveNotification(SL("Could not find a sim account, can't open chat. Please check the log for details"), Utils::LongNotificationDuration);
        return;
    }

    // Look for an existing channel
    for (const auto &channelptr : qAsConst(m_channels)) {
        if (channelptr->targetId() == phoneNumber) {
            qDebug() << "found existing channel" << channelptr.data();
            emit channelOpen(channelptr, phoneNumber);
            return;
        }
    }

    // If there is none just ask for a new one
    Tp::PendingChannelRequest *pendingChannel = m_simAccount->ensureTextChat(phoneNumber, QDateTime::currentDateTime(), SL("org.freedesktop.Telepathy.Client.SpaceBar"));
    connect(pendingChannel, &Tp::PendingChannelRequest::finished, this, [=](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Requesting text channel failed:" << op->errorName() << op->errorMessage();
            Utils::instance()->showPassiveNotification(SL("Failed to request channel. Please check the log for details"), Utils::LongNotificationDuration);
            return;
        }

        auto *request = qobject_cast<Tp::PendingChannelRequest *>(op);
        if (request) {
            auto channel = Tp::TextChannelPtr::qObjectCast(request->channelRequest()->channel());
            if (channel) {
                m_channels.append(channel);
                emit channelOpen(channel, phoneNumber);
            }
            return;
        }
    });
}

Database *ChannelHandler::database() const
{
    return m_database;
}
