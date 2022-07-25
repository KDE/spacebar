// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channellogger.h"
#include "modemcontroller.h"
#include "settingsmanager.h"

#include <QTimer>

#include <KLocalizedString>
#include <KNotification>
#include <KPeople/PersonData>

#include <contactphonenumbermapper.h>
#include <global.h>

static bool isScreenSaverActive()
{
    bool active = false;
    QDBusMessage request = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("GetActive"));
    const QDBusReply<bool> response = QDBusConnection::sessionBus().call(request);
    active = response.isValid() ? response.value() : false;
    return active;
}

ChannelLogger::ChannelLogger(std::optional<QString> &modemPath, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Daemon"), this, QDBusConnection::ExportScriptableContents);

    ModemController::instance().init(modemPath);

    m_ownNumber = PhoneNumber(ModemController::instance().ownNumber());

    connect(&m_mms, &Mms::downloadFinished, this, &ChannelLogger::handleDownloadedMessage);

    connect(&m_mms, &Mms::downloadError, this, &ChannelLogger::createDownloadNotification);

    connect(&m_mms, &Mms::manualDownloadFinished, this, &ChannelLogger::manualDownloadFinished);

    checkMessages();

    connect(&ModemController::instance(), &ModemController::messageAdded, this, [this](ModemManager::Sms::Ptr msg) {
        handleIncomingMessage(msg);
    });

    connect(&ModemController::instance(), &ModemController::modemConnected, this, &ChannelLogger::checkMessages);

    connect(&ModemController::instance(), &ModemController::modemDataConnectedChanged, this, [this](const bool isConnected) {
        m_dataConnected = isConnected;

        if (isConnected) {
            for (const auto &indicator : m_deferredIndicators) {
                m_mms.sendNotifyResponse(indicator, SL("deferred"));
            }
            m_deferredIndicators.clear();
        }
    });
}

void ChannelLogger::checkMessages()
{
    // for any unhandled messages
    const ModemManager::Sms::List messages = ModemController::instance().messages();
    for (const ModemManager::Sms::Ptr &msg : messages) {
        if (msg->state() == MMSmsState::MM_SMS_STATE_RECEIVED) {
            handleIncomingMessage(msg);
        }
    }
}

void ChannelLogger::handleIncomingMessage(ModemManager::Sms::Ptr msg)
{
    const QString number = msg->number();
    const QString text = msg->text();
    const QDateTime datetime = msg->timestamp();
    const QByteArray data = msg->data();
    ModemController::instance().deleteMessage(msg->uni());

    const PhoneNumberList phoneNumberList = PhoneNumberList(number);

    //TODO check if blocked number
    // use phonebook for blocked number storage so can be shared with dialer?

    // text and data are not valid at the same time
    if (!text.isEmpty()) {
        saveMessage(phoneNumberList, datetime, text);
    } else if (!data.isEmpty()) {
        MmsMessage mmsMessage;
        m_mms.decodeNotification(mmsMessage, data);

        // should be in the notification itself, but adding here for redundancy
        if (mmsMessage.from.isEmpty()) {
            mmsMessage.from = number;
        }
        if (mmsMessage.date.isNull()) {
            mmsMessage.date = datetime;
        }

        if (mmsMessage.messageType == SL("m-notification-ind")) {
            if (mmsMessage.contentType == SL("application/vnd.wap.mms-message")) {
                // give modem data an opportunity to connect if not currently connected
                QTimer::singleShot(1000 * (m_dataConnected ? 0 : 10), this, [this, mmsMessage]() {
                    bool autoDownload = SettingsManager::self()->autoDownload();
                    const bool autoDownloadContactsOnly = SettingsManager::self()->autoDownloadContactsOnly();
                    if (autoDownload && autoDownloadContactsOnly) {
                        const QString name = KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(PhoneNumber(mmsMessage.from)), this).name();
                        if (name.isEmpty()) {
                            autoDownload = false;
                        }
                    }
                    if (autoDownload && m_dataConnected) {
                        m_mms.downloadMessage(mmsMessage);
                    } else {
                        // manually download later
                        createDownloadNotification(mmsMessage);
                    }
                });
            } else {
                qDebug() << "Unknown content type:" << mmsMessage.contentType;
            }
        } else if (mmsMessage.messageType == SL("m-delivery-ind")) {
            if (!mmsMessage.messageId.isEmpty()) {
                m_database.updateMessageDeliveryReport(mmsMessage.messageId);
            }
        } else if (mmsMessage.messageType == SL("m-read-orig-ind"))  {
            if (!mmsMessage.messageId.isEmpty()) {
                m_database.updateMessageReadReport(mmsMessage.messageId, PhoneNumber(mmsMessage.from));
            }
        } else if (mmsMessage.messageType == SL("m-cancel-req"))  {
            m_mms.sendCancelResponse(mmsMessage.transactionId);
        } else {
            qDebug() << "Unknown message type:" << mmsMessage.messageType;
            m_mms.sendNotifyResponse(mmsMessage.transactionId, SL("unrecognized"));
        }
    } else {
        saveMessage(phoneNumberList, datetime);
    }
}

void ChannelLogger::createDownloadNotification(const MmsMessage &mmsMessage)
{
    saveMessage(
        PhoneNumberList(mmsMessage.from),
        mmsMessage.date,
        QString(), // text
        QString(), // attachments
        QString(), // smil
        QString(), // fromNumber
        QString(), // messageId
        true, // pendingDownload
        mmsMessage.contentLocation,
        mmsMessage.expiry,
        mmsMessage.messageSize
    );

    // this is important, otherwise an MMSC server may send repeated notifications
    if (m_dataConnected) {
        m_mms.sendNotifyResponse(mmsMessage.transactionId, SL("deferred"));
    } else {
        m_deferredIndicators.append(mmsMessage.transactionId);
    }
}

void ChannelLogger::manualDownload(const QString &id, const QString &url, const QDateTime &expires)
{
    MmsMessage mmsMessage;
    mmsMessage.databaseId = id;
    mmsMessage.contentLocation = url;
    mmsMessage.expiry = expires;
    mmsMessage.transactionId = m_mms.generateTransactionId();

    if (m_dataConnected) {
        m_mms.downloadMessage(mmsMessage);
    } else {
        Q_EMIT manualDownloadFinished(id, true);
    }
}

void ChannelLogger::handleDownloadedMessage(const QByteArray &response, const QString &url, const QDateTime &expires)
{
    MmsMessage mmsMessage;
    mmsMessage.ownNumber = m_ownNumber;
    m_mms.decodeMessage(mmsMessage, response);

    // fromNumber is only useful to know in group conversations
    const QString fromNumber = mmsMessage.to.length() > 1 ? mmsMessage.from : QString();

    saveMessage(
        mmsMessage.phoneNumberList,
        mmsMessage.date,
        mmsMessage.text,
        mmsMessage.attachments,
        mmsMessage.smil,
        fromNumber,
        mmsMessage.messageId,
        false,
        url,
        expires,
        response.size()
    );
}

void ChannelLogger::saveMessage(
    const PhoneNumberList &phoneNumberList,
    const QDateTime &datetime,
    const QString &text,
    const QString &attachments,
    const QString &smil,
    const QString &fromNumber,
    const QString &messageId,
    const bool pendingDownload,
    const QString &contentLocation,
    const QDateTime &expires,
    const int size
)
{
    Message message;
    message.text = text;
    message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
    message.datetime = datetime;
    message.deliveryStatus =  MessageState::Received; // It arrived, soo
    message.phoneNumberList = phoneNumberList;
    message.id = Database::generateRandomId();
    message.read = message.phoneNumberList == m_disabledNotificationNumber;
    message.attachments = attachments;
    message.smil = smil;
    message.fromNumber = fromNumber;
    message.messageId = messageId;
    message.pendingDownload = pendingDownload;
    message.contentLocation = contentLocation;
    message.expires = expires;
    message.size = size;

    m_database.addMessage(message);

    Q_EMIT messageAdded(message.phoneNumberList.toString(), message.id);

    //TODO add setting to turn off notifications for multiple chats in addition to current chat
    if (message.phoneNumberList == m_disabledNotificationNumber) {
        if (!isScreenSaverActive()) {
            return;
        }
    }

    QString notificationText;
    if (SettingsManager::self()->showMessageContent()) {
        notificationText = text;
        notificationText.truncate(200);
        if (!attachments.isEmpty()) {
            int attachmentCount = attachments.count(SL("fileName"));
            notificationText = QString::number(attachmentCount) + SL(" ");
            notificationText.append(attachmentCount > 1 ? i18n("Attachments") : i18n("Attachment"));
        }
    }

    QString title = i18n("New message");
    if (SettingsManager::self()->showSenderInfo()) {
        const PhoneNumber from = fromNumber.isEmpty() ? phoneNumberList.first() : PhoneNumber(fromNumber);
        title = KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(from), this).name();
        if (title.isEmpty()) {
            title = from.toNational();
        }
        title = i18n("Message from %1", title);
    }

    auto *notification = new KNotification(QStringLiteral("incomingMessage"));
    notification->setComponentName(SL("spacebar"));
    notification->setIconName(SL("org.kde.spacebar"));
    notification->setTitle(title);
    notification->setText(notificationText);
    notification->setDefaultAction(i18nc("@action open message in application", "Open"));
    notification->sendEvent();

    // copy current pointer to notification, otherwise this would just close the most recent one.
    connect(notification, &KNotification::defaultActivated, this, [notification]() {
        notification->close();
        QProcess::startDetached(SL("spacebar"), QStringList{});
    });
}

void ChannelLogger::disableNotificationsForNumber(const QString &numbers)
{
    m_disabledNotificationNumber = PhoneNumberList(numbers);
}

void ChannelLogger::syncSettings()
{
    SettingsManager::self()->load();
}
