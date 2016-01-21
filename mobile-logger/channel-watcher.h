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

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/types.h>
#include <KTp/contact.h>
#include <KTp/message.h>

#include <QObject>

class ChannelWatcher : public QObject, public Tp::RefCounted
{
    Q_OBJECT

public:
    ChannelWatcher(const Tp::TextChannelPtr &channel, QObject *parent=0);
    ~ChannelWatcher();
    int unreadMessageCount() const;
    QString lastMessage() const;
    KTp::Message::MessageDirection lastMessageDirection() const;

Q_SIGNALS:
    void messagesChanged();
    void invalidated();

private Q_SLOTS:
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::Message &message);

private:
    Tp::TextChannelPtr m_channel;
    QString m_lastMessage;
    KTp::Message::MessageDirection m_lastMessageDirection;
};

typedef Tp::SharedPtr<ChannelWatcher> ChannelWatcherPtr;
