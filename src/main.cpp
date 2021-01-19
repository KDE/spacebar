// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QtQml>
#include <QQuickWindow>

#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/ConnectionFactory>

// Models
#include "chatlistmodel.h"
#include "messagemodel.h"

#include "contactmapper.h"
#include "global.h"
#include "channelhandler.h"
#include "utils.h"
#include "avatarimageprovider.h"

constexpr auto APPLICATION_ID = "org.kde.spacebar";


Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(SL("KDE"));
    QCoreApplication::setOrganizationDomain(SL("kde.org"));
    QCoreApplication::setApplicationName(SL("spacebar"));

    KLocalizedString::setApplicationDomain("spacebar");

    QQmlApplicationEngine engine;

    Tp::registerTypes();

    // Create registrar
    auto accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
        Tp::Features({Tp::Account::FeatureCore}));
    auto connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
        Tp::Features({Tp::Connection::FeatureCore, Tp::Connection::FeatureSelfContact, Tp::Connection::FeatureConnected}));
    auto channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    auto contactFactory = Tp::ContactFactory::create({});
    channelFactory->addFeaturesForTextChats(Tp::Features({Tp::TextChannel::FeatureCore, Tp::TextChannel::FeatureMessageQueue, Tp::TextChannel::FeatureMessageSentSignal}));

    auto registrar = Tp::ClientRegistrar::create(accountFactory, connectionFactory,
        channelFactory, contactFactory);

    // Create client
    auto handler = ChannelHandlerPtr::dynamicCast(Tp::SharedPtr<ChannelHandler>(new ChannelHandler()));
    registrar->registerClient(handler, SL("Spacebar"));

    Q_ASSERT(handler->isRegistered());

    auto *chatListModel = new ChatListModel(handler, &engine);
    // Use using the instance getter
    new Utils(&engine);

    // Register types
    qmlRegisterSingletonInstance(APPLICATION_ID, 1, 0 , "ChatListModel", chatListModel);
    qmlRegisterUncreatableType<MessageModel>(APPLICATION_ID, 1, 0, "MessageModel", SL("Created by ChatListModel whenever a new chat was opened"));
    qRegisterMetaType<KPeople::PersonData *>("PersonData*");
    qmlRegisterAnonymousType<QAbstractItemModel>(APPLICATION_ID, 1);
    qmlRegisterSingletonInstance<Utils>(APPLICATION_ID, 1, 0, "Utils", Utils::instance());
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.addImageProvider(SL("avatar"), new AvatarImageProvider());
    engine.load(QUrl(SL("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
