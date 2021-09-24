// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QCommandLineParser>
#include <QCoreApplication>

#include <KLocalizedString>
#include <KNotification>
#include <KTextToHTML>

#include <ModemManagerQt/Sms>

#include "global.h"
#include "modemcontroller.h"

#include <database.h>
#include <global.h>
#include <phonenumberutils.h>

int main(int argc, char *argv[])
{
    QCommandLineParser parser;

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(SL("KDE"));
    QCoreApplication::setOrganizationDomain(SL("kde.org"));
    QCoreApplication::setApplicationName(SL("Spacebar"));

    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(i18n("Spacebar background service"));
    const auto modemOpt = QCommandLineOption(SL("modem"), SL("Modem path to use, for development purpose"), SL("modem"));
    parser.addOption(modemOpt);
    parser.process(app);

    // Create observer
    auto modemPath = parser.isSet(modemOpt) && !parser.value(modemOpt).isEmpty()
            ? parser.value(modemOpt)
            : std::optional<QString>();

    ModemController::instance().init(modemPath);

    Database database;

    QObject::connect(&ModemController::instance(), &ModemController::messageAdded, &app, [&database, &app](ModemManager::Sms::Ptr msg, bool received) {
        if (!received) {
            return;
        }

        Message message;
        message.text = KTextToHTML::convertToHtml(msg->text(), KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::ConvertPhoneNumbers));
        message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
        message.datetime = msg->timestamp();
        message.deliveryStatus =  MessageState::Received; // It arrived, soo
        message.phoneNumber = phoneNumberUtils::normalizeNumber(msg->number());
        message.id = Database::generateRandomId();
        message.read = false;

        database.addMessage(message);

        auto *notification = new KNotification(QStringLiteral("incomingMessage"));
        notification->setComponentName(SL("spacebar"));
        notification->setIconName(SL("org.kde.spacebar"));
        notification->setTitle(i18n("Message from %1", msg->number()));
        notification->setText(msg->text());
        notification->setDefaultAction(i18nc("@action open message in application", "Open"));
        notification->sendEvent();

        // copy current pointer to notification, otherwise this would just close the most recent one.
        QObject::connect(notification, &KNotification::defaultActivated, &app, [notification]() {
            notification->close();
            QProcess::startDetached(SL("spacebar"), QStringList{});
        });
    });

    QCoreApplication::exec();
}
