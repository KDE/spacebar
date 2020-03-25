#include "channelhandler.h"

#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ReceivedMessage>

ChannelHandler::ChannelHandler()
    : Tp::AbstractClientHandler(Tp::ChannelClassSpecList({
        Tp::ChannelClassSpec::textChat(), Tp::ChannelClassSpec::unnamedTextChat()
    }))
{
    // Set up sms account
    QEventLoop loop;
    Tp::AccountManagerPtr manager = Tp::AccountManager::create();
    Tp::PendingReady *ready = manager->becomeReady();
    QObject::connect(ready, &Tp::PendingReady::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

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
        if (!channel) {
            qDebug() << "channel is not a text channel. None of my business";
        }

        connect(textChannel.data(), &Tp::TextChannel::messageReceived, this, [](const Tp::ReceivedMessage &message) {
            qDebug() << "received message" << message.text();
        });
        qDebug() << "Found a new text channel, yay" << channel.data();
        if (m_channels.contains(textChannel)) {
            m_channels.append(textChannel);
        }
    }

    context->setFinished();
}

void ChannelHandler::openChannel(const QString &phoneNumber)
{
    // Look for an existing channel
    for (const auto &channelptr : m_channels) {
        if (channelptr.data()->targetId() == phoneNumber) {
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
            return;
        }

        auto *pc = qobject_cast<Tp::PendingChannel *>(op);
        if (pc) {
            auto channel = Tp::TextChannelPtr::dynamicCast(pc->channel());
            m_channels.append(channel);
            emit channelOpen(channel, phoneNumber);
            return;
        }
    });
}
