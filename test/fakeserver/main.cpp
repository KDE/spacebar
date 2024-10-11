// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>

#include <KAboutData>

#include "server.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // start wizard
    KAboutData aboutData(QStringLiteral("spacebar-fakeserver"),
                         QStringLiteral("Spacebar Fakeserver"),
                         QStringLiteral("1.0"),
                         QStringLiteral(""),
                         KAboutLicense::GPL,
                         QStringLiteral("© 2024 KDE Community"));
    aboutData.addAuthor(QStringLiteral("Devin Lin"), QString(), QStringLiteral("devin@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QQmlApplicationEngine engine;

    Server::instance();

    qmlRegisterSingletonType<Server>("spacebarfakeserver", 1, 0, "Server", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return Server::instance();
    });

    engine.load(QUrl(QStringLiteral("qrc:org/kde/spacebar/fakeserver/qml/Main.qml")));

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("start-here-symbolic")));

    return app.exec();
}
