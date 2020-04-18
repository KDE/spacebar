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

ChannelHandler::ChannelHandler()
    : Tp::AbstractClientHandler(Tp::ChannelClassSpecList({
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

        // FIXME Only gets second message, first is already gone at this point
        connect(textChannel.data(), &Tp::TextChannel::messageReceived, this, &ChannelHandler::handleIncomingMessage);
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
    Tp::PendingChannel *pendingChannel = m_simAccount->ensureAndHandleTextChat(phoneNumber);
    qDebug() << pendingChannel;
    connect(pendingChannel, &Tp::PendingChannel::finished, this, [=](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Requesting text channel failed:" << op->errorName() << op->errorMessage();
            Utils::instance()->showPassiveNotification(SL("Failed to request channel. Please check the log for details"), Utils::LongNotificationDuration);
            return;
        }

        auto *pc = qobject_cast<Tp::PendingChannel *>(op);
        if (pc) {
            auto channel = Tp::TextChannelPtr::qObjectCast(pc->channel());
            connect(channel.data(), &Tp::TextChannel::messageReceived, this, &ChannelHandler::handleIncomingMessage);

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

void ChannelHandler::handleIncomingMessage(const Tp::ReceivedMessage receivedMessage)
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
    m_database->addMessage(message);
}
