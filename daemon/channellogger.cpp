// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "channellogger.h"

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ReceivedMessage>

#include "global.h"
#include "database.h"

ChannelLogger::ChannelLogger(QObject *parent)
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

void ChannelLogger::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
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
        for (const auto &message : messageQueue) {
            handleIncomingMessage(textChannel, message);
        }

        connect(textChannel.data(), &Tp::TextChannel::messageReceived, this, [this, textChannel](const Tp::ReceivedMessage &receivedMessage) {
            handleIncomingMessage(textChannel, receivedMessage);
        });
        connect(textChannel.data(), &Tp::TextChannel::messageSent, this, [this, textChannel](const Tp::Message &sentMessage) {
            handleOutgoingMessage(textChannel, sentMessage);
        });

        qDebug() << "Found a new text channel, yay" << channel.data();
        if (!m_channels.contains(textChannel)) {
            m_channels.append(textChannel);
        }
    }

    context->setFinished();
}

void ChannelLogger::handleIncomingMessage(Tp::TextChannelPtr channel, const Tp::ReceivedMessage &receivedMessage)
{
    qDebug() << "received message" << receivedMessage.text();

    if (receivedMessage.isDeliveryReport()) {
        qDebug() << "received delivery report";
        // TODO: figure out correct ID and mark it as delivered.
        return;
    }

    Message message;
    message.text = receivedMessage.text();
    message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
    message.datetime = receivedMessage.received();
    message.delivered = true; // It arrived, soo
    message.phoneNumber = receivedMessage.sender()->id();
    message.id = m_database->lastId() + 1;
    message.read = false;

    qDebug() << "writing to db";
    m_database->addMessage(message);

    channel->acknowledge({receivedMessage});
}

void ChannelLogger::handleOutgoingMessage(Tp::TextChannelPtr channel, const Tp::Message &sentMessage)
{
    qDebug() << "sent message" << sentMessage.text();

    Message message;
    message.text = sentMessage.text();
    message.sentByMe = true; // it is outgoing
    message.datetime = sentMessage.sent();
    message.phoneNumber = channel->targetId();
    message.id = m_database->lastId() + 1;
    message.read = true; // sent by us, so already read by us

    qDebug() << "writing to db";
    m_database->addMessage(message);
}
