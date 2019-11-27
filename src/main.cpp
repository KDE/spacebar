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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlComponent>

#include <QStandardPaths>
#include <QDebug>

#include <KDBusService>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "EmojiModel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("SpaceBar");
    app.setOrganizationDomain("kde.org");

    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption("contact", i18n("Open with the conversation matching the contact id")));
    parser.addOption(QCommandLineOption("openIncomingChannel", i18n("If defined, it will automatically open the first handed channel")));
    parser.addHelpOption();
    parser.process(app);

    if (parser.positionalArguments().size() > 1) {
        parser.showHelp(1);
    }

    if (parser.isSet("contact")) {
        // TODO
    }

    qmlRegisterType<EmojiModel>("EmojiModel", 0, 1, "EmojiModel");
    qmlRegisterType<EmojiProxyModel>("EmojiModel", 0, 1, "EmojiProxyModel");
    qmlRegisterUncreatableType<QAbstractItemModel>("EmojiModel", 0, 1, "QAbstractItemModel", "Used by proxy models");
    qmlRegisterUncreatableType<Emoji>("EmojiModel", 0, 1, "Emoji", "Used by emoji models");

    QQmlApplicationEngine engine;

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    engine.rootContext()->setContextProperty("commandlineArguments", parser.positionalArguments());
    engine.rootContext()->setContextProperty("openIncomingChannel", parser.isSet("openIncomingChannel"));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
