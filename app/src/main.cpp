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

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>

#include <QStandardPaths>
#include <QDebug>
#include <QThread>

#include <KDBusService>
#include <KLocalizedString>
#include <KDeclarative/QmlObject>

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <iostream>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationDisplayName("SpaceBar");

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

    const QString packagePath("org.kde.spacebar");

    //usually we have an ApplicationWindow here, so we do not need to create a window by ourselves
    KDeclarative::QmlObject *obj = new KDeclarative::QmlObject();
    obj->setTranslationDomain(packagePath);
    obj->setInitializationDelayed(true);
    obj->loadPackage(packagePath);
    obj->engine()->rootContext()->setContextProperty("commandlineArguments", parser.positionalArguments());

    obj->engine()->rootContext()->setContextProperty("openIncomingChannel", parser.isSet("openIncomingChannel"));

    obj->completeInitialization();

    if (!obj->package().metadata().isValid()) {
        return -1;
    }

    QWindow *window = qobject_cast<QWindow *>(obj->rootObject());
    if (window) {
        window->show();
        window->requestActivate();
        window->setTitle(obj->package().metadata().name());
        window->setIcon(QIcon::fromTheme(obj->package().metadata().iconName()));
    }

    return app.exec();
}
