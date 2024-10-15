// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QtDBus/QDBusMetaType>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

#include "../version.h"
#include "channellogger.h"
#include "global.h"
#include <database.h>

using namespace Qt::Literals;

int main(int argc, char *argv[])
{
    QCommandLineParser parser;

    QCoreApplication app(argc, argv);

    // Ensure DBus service is registered as org.kde.spacebar.Daemon
    KAboutData about(u"spacebar.Daemon"_s, u"Spacebar"_s, QStringLiteral(SPACEBAR_VERSION_STRING));
    KAboutData::setApplicationData(about);

    KCrash::initialize();

    KLocalizedString::setApplicationDomain("spacebar");

    app.setQuitLockEnabled(false); // prevent a finishing KJob from closing the daemon

    qRegisterMetaType<StringMapList>();
    qDBusRegisterMetaType<StringMap>();
    qDBusRegisterMetaType<StringMapList>();

    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(i18n("Spacebar background service"));
    const auto modemOpt = QCommandLineOption(SL("modem"), SL("Modem path to use, for development purpose"), SL("modem"));
    parser.addOption(modemOpt);
    parser.process(app);

    KDBusService service(KDBusService::Unique);

    // Create observer
    auto modemPath = parser.isSet(modemOpt) && !parser.value(modemOpt).isEmpty() ? parser.value(modemOpt) : std::optional<QString>();

    ChannelLogger logger(modemPath);

    QCoreApplication::exec();
}
