// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowSystem>

#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QUrl>
#include <QtQml>

// Models
#include "chatlistmodel.h"
#include "messagemodel.h"

#include "about.h"
#include "avatarimageprovider.h"
#include "channelhandler.h"
#include "global.h"
#include "settingsmanager.h"
#include "utils.h"
#include "version.h"
#include <contactphonenumbermapper.h>

#include <coroutine>
#include <phonenumberlist.h>

constexpr auto APPLICATION_ID = "org.kde.spacebar";

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    QApplication app(argc, argv);

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE") && QQuickStyle::name().isEmpty()) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QCoreApplication::setOrganizationName(SL("KDE"));
    QCoreApplication::setOrganizationDomain(SL("kde.org"));
    QCoreApplication::setApplicationName(SL("spacebar"));
    QCoreApplication::setApplicationVersion(QStringLiteral(SPACEBAR_VERSION_STRING));
    QGuiApplication::setApplicationDisplayName(SL("Spacebar"));

    KLocalizedString::setApplicationDomain("spacebar");

    KAboutData about(SL("spacebar"),
                     SL("Spacebar"),
                     QStringLiteral(SPACEBAR_VERSION_STRING),
                     i18n("SMS/MMS messaging client"),
                     KAboutLicense::GPL,
                     i18n("© 2020-2021 KDE Community"));
    about.addAuthor(i18n("Bhushan Shah"), QString(), QStringLiteral("bhush94@gmail.com"));
    about.addAuthor(i18n("Jonah Brüchert"), QString(), QStringLiteral("jbb@kaidan.im"));
    about.addAuthor(i18n("Martin Klapetek"), QString(), QStringLiteral("mklapetek@kde.org"));
    about.addAuthor(i18n("Michael Lang"), QString(), QStringLiteral("criticaltemp@protonmail.com"));
    about.addAuthor(i18n("Nicolas Fella"), QString(), QStringLiteral("nicolas.fella@gmx.de"));
    about.addAuthor(i18n("Smitty van Bodegom"), QString(), QStringLiteral("me@smitop.com"));
    KAboutData::setApplicationData(about);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.spacebar")));

    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(i18n("Spacebar SMS/MMS client"));
    parser.addPositionalArgument(QStringLiteral("number"), i18n("Open a chat with the given phone number"));
    parser.process(app);

    KDBusService service(KDBusService::Unique);

    QQmlApplicationEngine engine;

    // Use using the instance getter
    new Utils(&engine);

    ChannelHandler handler;
    ChatListModel chatListModel(handler);

    // Register types
    qmlRegisterSingletonInstance("org.kde.spacebar", 1, 0, "AboutType", &AboutType::instance());
    qmlRegisterSingletonInstance<ChatListModel>(APPLICATION_ID, 1, 0, "ChatListModel", &chatListModel);
    qmlRegisterUncreatableType<MessageModel>(APPLICATION_ID, 1, 0, "MessageModel", SL("Created by ChatListModel whenever a new chat was opened"));
    qRegisterMetaType<KPeople::PersonData *>("PersonData*");
    qmlRegisterAnonymousType<QAbstractItemModel>(APPLICATION_ID, 1);
    qmlRegisterSingletonInstance<Utils>(APPLICATION_ID, 1, 0, "Utils", Utils::instance());
    qmlRegisterSingletonInstance("org.kde.spacebar", 1, 0, "SettingsManager", SettingsManager::self());
    qRegisterMetaType<PhoneNumber>();
    qRegisterMetaType<PhoneNumberList>();
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.addImageProvider(SL("avatar"), new AvatarImageProvider());
    engine.load(QUrl(SL("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    auto handleArgs = [&parser, &chatListModel] {
        if (!parser.positionalArguments().isEmpty()) {
            QString numberArg = parser.positionalArguments().constFirst();
            if (numberArg.startsWith(QStringLiteral("sms:"))) {
                numberArg = numberArg.mid(4);
            }
            if (Utils::instance()->isPhoneNumber(numberArg)) {
                chatListModel.startChat(PhoneNumberList(numberArg));
            } else {
                qWarning() << "invalid phone number on command line, ignoring";
            }
        }
    };

    QObject::connect(&service, &KDBusService::activateRequested, &service, [&parser, &handleArgs, &engine](const QStringList &args) {
        parser.parse(args);
        handleArgs();

        // Move window to the front
        auto *window = qobject_cast<QWindow *>(engine.rootObjects().constFirst());
        if (window) {
            window->hide(); // Hack - remove once works correctly without
            KWindowSystem::updateStartupId(window);
            KWindowSystem::activateWindow(window);
            window->show(); // Hack - remove once works correctly without
        }
    });

    handleArgs();

    return app.exec();
}
