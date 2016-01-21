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

ChannelWatcher::ChannelWatcher(const Tp::TextChannelPtr &channel, QObject *parent)
    : QObject(parent),
      m_channel(channel),
      m_lastMessageDirection(KTp::Message::LocalToRemote)
{
    connect(channel.data(), &Tp::TextChannel::pendingMessageRemoved, [=](Tp::ReceivedMessage message) {
        qDebug() << "Message removed" << message.text();
    });
    connect(channel.data(), &Tp::TextChannel::invalidated, this, &ChannelWatcher::invalidated);
    connect(channel.data(), &Tp::TextChannel::invalidated, this, [=]() {
        qDebug() << "Channel invalidated";
    });

    connect(channel.data(), SIGNAL(messageReceived(Tp::ReceivedMessage)), SLOT(onMessageReceived(Tp::ReceivedMessage)));
    connect(channel.data(), SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)), SLOT(onMessageSent(Tp::Message)));

    //trigger an update to the contact straight away
    QTimer::singleShot(0, this, SIGNAL(messagesChanged()));

    qDebug() << this << "New channel being watched" << channel.data();

}

ChannelWatcher::~ChannelWatcher()
{
}

int ChannelWatcher::unreadMessageCount() const
{
    return m_channel->messageQueue().size();
}

QString ChannelWatcher::lastMessage() const
{
    return m_lastMessage;
}

KTp::Message::MessageDirection ChannelWatcher::lastMessageDirection() const
{
    return m_lastMessageDirection;
}

void ChannelWatcher::onMessageReceived(const Tp::ReceivedMessage &message)
{
    if (!message.isDeliveryReport()) {
        m_lastMessage = message.text();
        m_lastMessageDirection = KTp::Message::RemoteToLocal;
        Q_EMIT messagesChanged();
    }

    qDebug() << m_lastMessage;
}

void ChannelWatcher::onMessageSent(const Tp::Message &message)
{
    m_lastMessage = message.text();
    m_lastMessageDirection = KTp::Message::LocalToRemote;
    qDebug() << m_lastMessage;
    Q_EMIT messagesChanged();
}
