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

#include <KLocalizedString>
#include <KNotification>

#include <global.h>
#include <database.h>
#include <accountutils.h>
#include <phonenumberutils.h>

ChannelLogger::ChannelLogger(QObject *parent)
    : QObject(parent)
    , Tp::AbstractClientObserver(Tp::ChannelClassSpecList({
          Tp::ChannelClassSpec::textChat(), Tp::ChannelClassSpec::unnamedTextChat()
      }))
    , m_database(new Database(this))
{
    // Set up sms account
    Tp::AccountManagerPtr manager = Tp::AccountManager::create();
    Tp::PendingReady *ready = manager->becomeReady();
    QObject::connect(ready, &Tp::PendingReady::finished, this, [=] {
        m_simAccount = *AccountUtils::findTelephonyAccount(manager);
        emit handlerReady();
    });
}

void ChannelLogger::observeChannels(const Tp::MethodInvocationContextPtr<> &context, const Tp::AccountPtr & /*account*/, const Tp::ConnectionPtr & /*connection*/, const QList<Tp::ChannelPtr> &channels, const Tp::ChannelDispatchOperationPtr & /*dispatchOperation*/, const QList<Tp::ChannelRequestPtr> & /*requestsSatisfied*/, const Tp::AbstractClientObserver::ObserverInfo & /*observerInfo*/)
{
    qDebug() << "observer called";
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

void ChannelLogger::handleIncomingMessage(const Tp::TextChannelPtr& /* channel */, const Tp::ReceivedMessage &receivedMessage)
{
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
    message.phoneNumber = PhoneNumberUtils::normalize(receivedMessage.sender()->id());
    message.id = m_database->lastId() + 1;
    message.read = false;

    m_database->addMessage(message);

    auto *notification = new KNotification(QStringLiteral("incomingMessage"), KNotification::Persistent);
    notification->setComponentName(SL("spacebar"));
    notification->setIconName(SL("org.kde.spacebar"));
    notification->setTitle(i18n("Message from %1", receivedMessage.sender()->id()));
    notification->setText(receivedMessage.text());
    notification->setDefaultAction(i18nc("@action open message in application", "Open"));
    notification->sendEvent();

    // copy current pointer to notification, otherwise this would just close the most recent one.
    connect(notification, &KNotification::defaultActivated, this, [notification]() {
        notification->close();
        QProcess::startDetached(SL("spacebar"), QStringList{});
    });
}

void ChannelLogger::handleOutgoingMessage(Tp::TextChannelPtr channel, const Tp::Message &sentMessage)
{
    Message message;
    message.text = sentMessage.text();
    message.sentByMe = true; // it is outgoing
    message.datetime = sentMessage.sent();
    message.phoneNumber = channel->targetId();
    message.id = m_database->lastId() + 1;
    message.read = true; // sent by us, so already read by us

    m_database->addMessage(message);
}
