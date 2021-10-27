// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channellogger.h"
#include "modemcontroller.h"

#include <KLocalizedString>
#include <KNotification>
#include <KTextToHTML>

#include <global.h>
#include <phonenumberutils.h>

ChannelLogger::ChannelLogger(std::optional<QString> &modemPath, QObject *parent)
: QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Daemon"), this, QDBusConnection::ExportScriptableContents);

    ModemController::instance().init(modemPath);

    connect(&ModemController::instance(), &ModemController::messageAdded, this, [this](ModemManager::Sms::Ptr msg) {
        handleIncomingMessage(msg);
    });
}

void ChannelLogger::handleIncomingMessage(ModemManager::Sms::Ptr msg)
{
    Message message;
    message.text = KTextToHTML::convertToHtml(msg->text(), KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::ConvertPhoneNumbers));
    message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
    message.datetime = msg->timestamp();
    message.deliveryStatus =  MessageState::Received; // It arrived, soo
    message.phoneNumber = phoneNumberUtils::normalizeNumber(msg->number());
    message.id = Database::generateRandomId();
    message.read = false;

    m_database.addMessage(message);

    ModemController::instance().deleteMessage(msg->uni());

    //TODO add setting to turn off notifications for multiple chats in addition to current chat
    if (message.phoneNumber == m_disabledNotificationNumber) {
        return;
    }

    auto *notification = new KNotification(QStringLiteral("incomingMessage"));
    notification->setComponentName(SL("spacebar"));
    notification->setIconName(SL("org.kde.spacebar"));
    notification->setTitle(i18n("Message from %1", msg->number()));
    notification->setText(msg->text());
    notification->setDefaultAction(i18nc("@action open message in application", "Open"));
    notification->sendEvent();

    // copy current pointer to notification, otherwise this would just close the most recent one.
    connect(notification, &KNotification::defaultActivated, this, [notification]() {
        notification->close();
        QProcess::startDetached(SL("spacebar"), QStringList{});
    });
}

void ChannelLogger::disableNotificationsForNumber(const QString &phoneNumber)
{
    m_disabledNotificationNumber = phoneNumber;
}
