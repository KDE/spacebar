// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channellogger.h"

#include <KLocalizedString>
#include <KNotification>

#include <KTextToHTML>

#include <qofonomessage.h>

#include <global.h>
#include <database.h>
#include <phonenumberutils.h>

ChannelLogger::ChannelLogger(std::optional<QString> &modemPath, QObject *parent)
    : QObject(parent)
{
    if (modemPath.has_value()) {
        m_msgManager.setModemPath(*modemPath);
    } else {
        connect(&m_manager, &QOfonoManager::defaultModemChanged, this, [&] {
            m_msgManager.setModemPath(m_manager.defaultModem());
        });

        m_msgManager.setModemPath(m_manager.defaultModem());
    }

    connect(&m_msgManager, &QOfonoMessageManager::incomingMessage, this, &ChannelLogger::handleIncomingMessage);
}

void ChannelLogger::handleIncomingMessage(const QString &text, const QVariantMap &info)
{
    Message message;
    message.text = KTextToHTML::convertToHtml(text, KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::ConvertPhoneNumbers));
    message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
    message.datetime = QDateTime::fromString(info[SL("SentTime")].toString().split(QChar(u'+'))[0], Qt::ISODate);
    message.deliveryStatus =  MessageState::Received; // It arrived, soo
    message.phoneNumber = PhoneNumberUtils::normalize(info[SL("Sender")].toString());
    message.id = Database::generateRandomId();
    message.read = false;

    m_database.addMessage(message);

    auto *notification = new KNotification(QStringLiteral("incomingMessage"));
    notification->setComponentName(SL("spacebar"));
    notification->setIconName(SL("org.kde.spacebar"));
    notification->setTitle(i18n("Message from %1", info[SL("Sender")].toString()));
    notification->setText(text);
    notification->setDefaultAction(i18nc("@action open message in application", "Open"));
    notification->sendEvent();

    // copy current pointer to notification, otherwise this would just close the most recent one.
    connect(notification, &KNotification::defaultActivated, this, [notification]() {
        notification->close();
        QProcess::startDetached(SL("spacebar"), QStringList{});
    });
}
