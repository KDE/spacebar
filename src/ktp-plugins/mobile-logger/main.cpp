/*
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
#include "text-channel-observer.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/ConnectionFactory>
#include <TelepathyQt/ChannelFactory>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/AccountManager>

#include <KTp/contact-factory.h>

class ObserverApp
{
public:
    Tp::ClientRegistrarPtr clientRegistrar;
    Tp::SharedPtr<KTp::TextChannelObserver> channelWatcherObserver;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ObserverApp *oa = new ObserverApp();

    Tp::registerTypes();

    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureSelfContact
                                                                               << Tp::Connection::FeatureRoster);

    Tp::ContactFactoryPtr contactFactory = KTp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                        << Tp::Contact::FeatureAvatarData
                                                                        << Tp::Contact::FeatureSimplePresence
                                                                        << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());





    Tp::AccountManagerPtr m_accountManager = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);
    QObject::connect(m_accountManager->becomeReady(), &Tp::PendingOperation::finished, &app, [=](Tp::PendingOperation *op) {

        qDebug() << "Manager ready, setting client registrar";

        Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
        channelFactory->addFeaturesForTextChats(Tp::Features() << Tp::Channel::FeatureCore << Tp::TextChannel::FeatureMessageQueue << Tp::TextChannel::FeatureMessageSentSignal << Tp::TextChannel::FeatureMessageCapabilities);

        oa->channelWatcherObserver = Tp::SharedPtr<KTp::TextChannelObserver>(new KTp::TextChannelObserver());

        oa->clientRegistrar = Tp::ClientRegistrar::create(m_accountManager->accountFactory(),
                                                                             m_accountManager->connectionFactory(),
                                                                             channelFactory,
                                                                             m_accountManager->contactFactory());

        oa->clientRegistrar->registerClient(Tp::AbstractClientPtr::dynamicCast(oa->channelWatcherObserver), QLatin1String("KTp.Mobile.Logger"));
    });

    return app.exec();
}
