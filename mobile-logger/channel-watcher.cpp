/*
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "channel-watcher.h"

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/types.h>
#include <KTp/contact.h>
#include <KTp/message.h>

ChannelWatcher::ChannelWatcher(const Tp::TextChannelPtr &channel, const QString &accountObjectPath, QObject *parent)
    : QObject(parent),
      m_channel(channel),
      m_accountObjectPath(accountObjectPath)
{
    connect(channel.data(), &Tp::TextChannel::invalidated, this, &ChannelWatcher::invalidated);
    connect(channel.data(), &Tp::TextChannel::invalidated, this, [=]() {
        qDebug() << "Channel invalidated";
    });

    connect(channel.data(), &Tp::TextChannel::messageReceived, this, &ChannelWatcher::onMessageReceived);
    connect(channel.data(), &Tp::TextChannel::messageSent, this, &ChannelWatcher::onMessageSent);

    qDebug() << this << "New channel being watched" << channel.data();
}

ChannelWatcher::~ChannelWatcher()
{
}

void ChannelWatcher::onMessageReceived(const Tp::ReceivedMessage &message)
{
    if (!message.isDeliveryReport()) {
        StorageMessage msg;
        msg.messageDateTime = message.received();
        msg.accountObjectPath = m_accountObjectPath;
        msg.targetContact = message.sender()->id();
        msg.message = message.text();
        msg.isIncoming = true;
        msg.type = 1;

        Q_EMIT storeMessage(msg);
    } else {

    }
}

void ChannelWatcher::onMessageSent(const Tp::Message &message)
{
    StorageMessage msg;
    msg.messageDateTime = message.sent();
    msg.accountObjectPath = m_accountObjectPath;
    msg.targetContact = m_channel->targetContact()->id();
    msg.message = message.text();
    msg.isIncoming = false;
    msg.type = 1;

    Q_EMIT storeMessage(msg);
}
