// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QtQml>
#include <QQuickWindow>
#include <QCommandLineParser>

// Models
#include "chatlistmodel.h"
#include "messagemodel.h"

#include <contactphonenumbermapper.h>
#include "version.h"
#include "global.h"
#include "utils.h"
#include "avatarimageprovider.h"
#include "channelhandler.h"

constexpr auto APPLICATION_ID = "org.kde.spacebar";


Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(SL("KDE"));
    QCoreApplication::setOrganizationDomain(SL("kde.org"));
    QCoreApplication::setApplicationName(SL("spacebar"));
    QCoreApplication::setApplicationVersion(QStringLiteral(SPACEBAR_VERSION_STRING));
    QGuiApplication::setApplicationDisplayName(SL("Spacebar"));

    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    KLocalizedString::setApplicationDomain("spacebar");

    QQmlApplicationEngine engine;

    // Use using the instance getter
    new Utils(&engine);

    ChannelHandler handler;
    ChatListModel chatListModel(handler);

    // Register types
    qmlRegisterSingletonInstance<ChatListModel>(APPLICATION_ID, 1, 0, "ChatListModel", &chatListModel);
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
