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
#include "contactmodel.h"

#include <contactphonenumbermapper.h>
#include "version.h"
#include "global.h"
#include "utils.h"
#include "avatarimageprovider.h"
#include "channelhandler.h"

#include <coroutine>

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
    parser.setApplicationDescription(i18n("Spacebar SMS client"));
    parser.addPositionalArgument(QStringLiteral("number"), i18n("Open a chat with the given phone number"));
    const auto modemOpt = QCommandLineOption(SL("modem"), SL("Modem path to use, for development purpose"), SL("modem"));
    parser.addOption(modemOpt);
    parser.process(app);

    KLocalizedString::setApplicationDomain("spacebar");

    QQmlApplicationEngine engine;

    // Use using the instance getter
    new Utils(&engine);

    auto modemPath = parser.isSet(modemOpt) && !parser.value(modemOpt).isEmpty()
            ? parser.value(modemOpt)
            : std::optional<QString>();
    ChannelHandler handler(modemPath);
    ChatListModel chatListModel(handler);

    // Register types
    qmlRegisterSingletonInstance<ChatListModel>(APPLICATION_ID, 1, 0, "ChatListModel", &chatListModel);
    qmlRegisterUncreatableType<MessageModel>(APPLICATION_ID, 1, 0, "MessageModel", SL("Created by ChatListModel whenever a new chat was opened"));
    qRegisterMetaType<KPeople::PersonData *>("PersonData*");
    qmlRegisterAnonymousType<QAbstractItemModel>(APPLICATION_ID, 1);
    qmlRegisterSingletonInstance<Utils>(APPLICATION_ID, 1, 0, "Utils", Utils::instance());
    qmlRegisterType<ContactModel>(APPLICATION_ID, 1, 0, "ContactModel");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.addImageProvider(SL("avatar"), new AvatarImageProvider());
    engine.load(QUrl(SL("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (!parser.positionalArguments().isEmpty()) {
        QString numberArg = parser.positionalArguments().constFirst();
        if (numberArg.startsWith(QStringLiteral("sms:"))) {
            numberArg = numberArg.mid(4);
        }
        if (Utils::instance()->isPhoneNumber(numberArg)) {
            chatListModel.startChat(numberArg);
        } else {
            qWarning() << "invalid phone number on command line, ignoring";
        }
    }

    return app.exec();
}
