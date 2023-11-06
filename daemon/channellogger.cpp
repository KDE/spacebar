// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "channellogger.h"
#include "modemcontroller.h"
#include "settingsmanager.h"

#include <QCoroFuture>
#include <QDBusConnection>
#include <QDBusReply>
#include <QLocale>
#include <QTimer>
#include <QtConcurrent>

#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KNotification>
#include <KPeople/PersonData>

#include <contactphonenumbermapper.h>
#include <global.h>

#include <QCoroDBusPendingReply>
#include <QCoroFuture>

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

    PhoneNumber::setCountryCode(countryCode());

    m_database.migrate();

    connect(&ModemController::instance(), &ModemController::messageAdded, this, [this](ModemManager::Sms::Ptr msg) {
        handleIncomingMessage(msg);
    });

    connect(&ModemController::instance(), &ModemController::modemConnected, this, &ChannelLogger::checkMessages);

    connect(&ModemController::instance(), &ModemController::modemDataConnectedChanged, this, [this](const bool isConnected) {
        m_dataConnected = isConnected;

        if (isConnected) {
            for (const auto &indicator : m_deferredIndicators) {
                sendNotifyResponse(indicator, SL("deferred"));
            }
            m_deferredIndicators.clear();
        }
    });

    connect(&ModemController::instance(), &ModemController::countryCodeChanged, this, [](const QString &countryCode) {
        PhoneNumber::setCountryCode(countryCode);
    });
}

void ChannelLogger::checkMessages()
{
    // update own number
    m_ownNumber = PhoneNumber(ownNumber());

    // for any unhandled messages
    const ModemManager::Sms::List messages = ModemController::instance().messages();
    for (const ModemManager::Sms::Ptr &msg : messages) {
        if (msg->state() == MMSmsState::MM_SMS_STATE_RECEIVED) {
            handleIncomingMessage(msg);
        }
    }
}

QString ChannelLogger::ownNumber()
{
    return ModemController::instance().ownNumber();
}

QString ChannelLogger::countryCode()
{
    QString countryCode = ModemController::instance().countryCode;

    if (countryCode.isEmpty()) {
        const QLocale locale;
        const QStringList qcountry = locale.name().split(u'_');
        countryCode = qcountry.constLast();
    }

    return countryCode;
}

void ChannelLogger::handleIncomingMessage(ModemManager::Sms::Ptr msg)
{
    const QString number = msg->number();
    const QString text = msg->text();
    const QDateTime datetime = msg->timestamp();
    const QByteArray data = msg->data();
    ModemController::instance().deleteMessage(msg->uni());

    const PhoneNumberList phoneNumberList = PhoneNumberList(number);

    // TODO check if blocked number
    //  use phonebook for blocked number storage so can be shared with dialer?

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
                        downloadMessage(mmsMessage);
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
        } else if (mmsMessage.messageType == SL("m-read-orig-ind")) {
            if (!mmsMessage.messageId.isEmpty()) {
                m_database.updateMessageReadReport(mmsMessage.messageId, PhoneNumber(mmsMessage.from));
            }
        } else if (mmsMessage.messageType == SL("m-cancel-req")) {
            sendCancelResponse(mmsMessage.transactionId);
        } else {
            qDebug() << "Unknown message type:" << mmsMessage.messageType;
            sendNotifyResponse(mmsMessage.transactionId, SL("unrecognized"));
        }
    } else {
        saveMessage(phoneNumberList, datetime);
    }
}

void ChannelLogger::createDownloadNotification(const MmsMessage &mmsMessage)
{
    saveMessage(PhoneNumberList(mmsMessage.from),
                mmsMessage.date,
                QString(), // text
                QString(), // attachments
                QString(), // smil
                QString(), // fromNumber
                QString(), // messageId
                true, // pendingDownload
                mmsMessage.contentLocation,
                mmsMessage.expiry,
                mmsMessage.messageSize);

    // this is important, otherwise an MMSC server may send repeated notifications
    if (m_dataConnected) {
        sendNotifyResponse(mmsMessage.transactionId, SL("deferred"));
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
        downloadMessage(mmsMessage);
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

    saveMessage(mmsMessage.phoneNumberList,
                mmsMessage.date,
                mmsMessage.text,
                mmsMessage.attachments,
                mmsMessage.smil,
                fromNumber,
                mmsMessage.messageId,
                false,
                url,
                expires,
                response.size());
}

QCoro::Task<void> ChannelLogger::addMessage(const Message &message)
{
    // save to database
    co_await m_database.addMessage(message);

    // add message to open conversation
    if (!message.sentByMe) {
        Q_EMIT messageAdded(message.phoneNumberList.toString(), message.id);
    }
}

void ChannelLogger::updateMessage(const Message &message)
{
    // update message in open conversation
    Q_EMIT messageUpdated(message.phoneNumberList.toString(), message.id);
}

QCoro::Task<void> ChannelLogger::saveMessage(const PhoneNumberList &phoneNumberList,
                                             const QDateTime &datetime,
                                             const QString &text,
                                             const QString &attachments,
                                             const QString &smil,
                                             const QString &fromNumber,
                                             const QString &messageId,
                                             const bool pendingDownload,
                                             const QString &contentLocation,
                                             const QDateTime &expires,
                                             const int size)
{
    Message message;
    message.text = text;
    message.sentByMe = false; // SMS doesn't have any kind of synchronization, so received messages are always from the chat partner.
    message.datetime = datetime;
    message.deliveryStatus = MessageState::Received; // It arrived, soo
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

    // prevent chronologically misordered chat history
    if (message.read && message.datetime.secsTo(QDateTime::currentDateTime()) < 60) {
        // adjust for small delays if conversation is currently open
        message.datetime = QDateTime::currentDateTime();
    } else if (message.datetime.daysTo(QDateTime::currentDateTime()) > 7) {
        // probably an invalid date if more than a week old
        message.datetime = QDateTime::currentDateTime();
    } else if (message.datetime > QDateTime::currentDateTime() && QDateTime::currentSecsSinceEpoch() > 31536000) {
        // future datetimes do not make sense
        message.datetime = QDateTime::currentDateTime();
    }

    if (co_await handleTapbackReaction(message, message.fromNumber.isEmpty() ? message.phoneNumberList.toString() : message.fromNumber)) {
        updateMessage(message);

        if (SettingsManager::self()->ignoreTapbacks()) {
            co_return;
        }
    } else {
        co_await addMessage(message);
    }

    // TODO add setting to turn off notifications for multiple chats in addition to current chat
    if (message.phoneNumberList == m_disabledNotificationNumber) {
        if (!isScreenSaverActive()) {
            co_return;
        }
    }

    createNotification(message);
}

void ChannelLogger::sendMessage(const QString &numbers, const QString &id, const QString &text, const QStringList &files, const qint64 &totalSize)
{
    PhoneNumberList phoneNumberList = PhoneNumberList(numbers);

    [this, phoneNumberList, id, text, files, totalSize]() -> QCoro::Task<void> {
        QString result;
        // check if it is a MMS message
        if (phoneNumberList.size() > 1 || files.length() > 0) {
            if (SettingsManager::self()->groupConversation()) {
                sendMessageMMS(phoneNumberList, id, text, files, totalSize);
            } else {
                // send as individual messages
                for (const auto &phoneNumber : phoneNumberList) {
                    if (files.length() > 0) {
                        sendMessageMMS(PhoneNumberList(phoneNumber.toInternational()), Database::generateRandomId(), text, files, totalSize);
                    } else {
                        result = co_await sendMessageSMS(phoneNumber, Database::generateRandomId(), text);
                    }
                }

                // update delivery status of original message
                Message message;
                message.id = id;
                message.phoneNumberList = phoneNumberList;
                message.datetime = QDateTime::currentDateTime();
                message.deliveryStatus = MessageState::Sent;
                updateMessage(message);
            }
        } else {
            result = co_await sendMessageSMS(phoneNumberList.first(), id, text);
        }

        if (result.isEmpty()) {
            qDebug() << "Message sent successfully";
        } else {
            qDebug() << "Failed successfully" << result;
        }
    }();
}

void ChannelLogger::sendTapback(const QString &numbers, const QString &id, const QString &tapback, const bool &isRemoved)
{
    sendTapbackHandler(numbers, id, tapback, isRemoved);
}

QCoro::Task<QString> ChannelLogger::sendMessageSMS(const PhoneNumber &phoneNumber, const QString &id, const QString &text)
{
    ModemManager::ModemMessaging::Message m;
    m.number = phoneNumber.toE164();
    m.text = text;

    Message message;
    message.id = id;
    message.phoneNumberList = PhoneNumberList(phoneNumber.toInternational());
    message.text = text;
    message.datetime = QDateTime::currentDateTime();
    message.read = true;
    message.sentByMe = true;
    message.deliveryStatus = MessageState::Pending;

    // add message to database
    co_await addMessage(message);

    auto maybeReply = ModemController::instance().createMessage(m);

    if (!maybeReply) {
        m_database.updateMessageDeliveryState(message.id, MessageState::Failed);
        updateMessage(message);
        co_return QStringLiteral("No modem");
    }

    const QDBusReply<QDBusObjectPath> msgPathResult = co_await *maybeReply;

    if (!msgPathResult.isValid()) {
        m_database.updateMessageDeliveryState(message.id, MessageState::Failed);
        updateMessage(message);
        co_return msgPathResult.error().message();
    }

    ModemManager::Sms::Ptr mmMessage = QSharedPointer<ModemManager::Sms>::create(msgPathResult.value().path());

    connect(mmMessage.get(), &ModemManager::Sms::stateChanged, this, [mmMessage, message, this] {
        qDebug() << "state changed" << mmMessage->state();

        switch (mmMessage->state()) {
        case MM_SMS_STATE_SENT:
            // The message was successfully sent
            m_database.updateMessageDeliveryState(message.id, MessageState::Sent);
            updateMessage(message);
            break;
        case MM_SMS_STATE_RECEIVED:
            // The message has been completely received
            // Should not happen
            qWarning() << "Received a message we sent";
            break;
        case MM_SMS_STATE_RECEIVING:
            // The message is being received but is not yet complete
            // Should not happen
            qWarning() << "Receiving a message we sent";
            break;
        case MM_SMS_STATE_SENDING:
            // The message is queued for delivery
            m_database.updateMessageDeliveryState(message.id, MessageState::Pending);
            updateMessage(message);
            break;
        case MM_SMS_STATE_STORED:
            // The message has been neither received nor yet sent
            m_database.updateMessageDeliveryState(message.id, MessageState::Pending);
            updateMessage(message);
            break;
        case MM_SMS_STATE_UNKNOWN:
            // State unknown or not reportable
            m_database.updateMessageDeliveryState(message.id, MessageState::Unknown);
            updateMessage(message);
            break;
        }
    });

    connect(mmMessage.get(), &ModemManager::Sms::deliveryStateChanged, this, [=] {
        MMSmsDeliveryState state = mmMessage->deliveryState();
        // This is only applicable if the PDU type is MM_SMS_PDU_TYPE_STATUS_REPORT
        // TODO handle and store message delivery report state
        qDebug() << "deliverystate changed" << state;
    });

    QDBusReply<void> sendResult = co_await mmMessage->send();

    if (!sendResult.isValid()) {
        m_database.updateMessageDeliveryState(message.id, MessageState::Failed);
        updateMessage(message);

        co_return sendResult.error().message();
    }

    co_return QString();
}

QCoro::Task<QString>
ChannelLogger::sendMessageMMS(const PhoneNumberList &phoneNumberList, const QString &id, const QString &text, const QStringList &files, const qint64 totalSize)
{
    Message message;
    message.phoneNumberList = phoneNumberList;
    message.text = text;
    message.datetime = QDateTime::currentDateTime();
    message.read = true;
    message.sentByMe = true;
    message.deliveryStatus = MessageState::Pending;

    MmsMessage mmsMessage;
    mmsMessage.ownNumber = m_ownNumber;
    mmsMessage.from = m_ownNumber.toInternational();
    mmsMessage.to = phoneNumberList.toString().split(u'~');
    mmsMessage.text = message.text;
    QByteArray data;
    m_mms.encodeMessage(mmsMessage, data, files, totalSize);

    // update message with encoded content parts
    message.id = id;
    message.text = mmsMessage.text;
    message.attachments = mmsMessage.attachments;
    message.smil = mmsMessage.smil;
    updateMessage(message);

    // add message to database
    co_await addMessage(message);

    // send message
    const QByteArray response = co_await uploadMessage(data);
    if (response.isEmpty()) {
        m_database.updateMessageDeliveryState(message.id, MessageState::Failed);
        updateMessage(message);
    } else {
        MmsMessage mmsMessage;
        m_mms.decodeConfirmation(mmsMessage, response);
        if (mmsMessage.responseStatus == 0) {
            m_database.updateMessageDeliveryState(message.id, MessageState::Sent);
            updateMessage(message);

            if (!mmsMessage.messageId.isEmpty()) {
                m_database.updateMessageSent(message.id, mmsMessage.messageId, mmsMessage.contentLocation);
            }
        } else {
            m_database.updateMessageDeliveryState(message.id, MessageState::Failed);
            updateMessage(message);
            qDebug() << mmsMessage.responseText;
        }
    }

    co_return QString();
}

QCoro::Task<void> ChannelLogger::sendCancelResponse(const QString &transactionId)
{
    const QByteArray data = m_mms.encodeCancelResponse(transactionId);
    const QByteArray response = co_await uploadMessage(data);
}

QCoro::Task<void> ChannelLogger::sendDeliveryAcknowledgement(const QString &transactionId)
{
    const QByteArray data = m_mms.encodeDeliveryAcknowledgement(transactionId);
    const QByteArray response = co_await uploadMessage(data);
}

QCoro::Task<void> ChannelLogger::sendNotifyResponse(const QString &transactionId, const QString &status)
{
    const QByteArray data = m_mms.encodeNotifyResponse(transactionId, status);
    const QByteArray response = co_await uploadMessage(data);
}

QCoro::Task<void> ChannelLogger::sendReadReport(const QString &messageId)
{
    const QByteArray data = m_mms.encodeReadReport(messageId);
    const QByteArray response = co_await uploadMessage(data);
}

void ChannelLogger::createNotification(Message &message)
{
    auto *notification = new KNotification(QStringLiteral("incomingMessage"));
    notification->setComponentName(SL("spacebar"));
    notification->setIconName(SL("message-new"));

    QString title = i18n("New message");
    if (SettingsManager::self()->showSenderInfo()) {
        const PhoneNumber from = message.fromNumber.isEmpty() ? message.phoneNumberList.first() : PhoneNumber(message.fromNumber);
        title = KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(from), this).name();
        if (title.isEmpty()) {
            title = from.toNational();
        }
        title = i18n("Message from %1", title);
    }
    notification->setTitle(title);

    if (SettingsManager::self()->showMessageContent()) {
        QString notificationText = message.text;
        notificationText.truncate(200);
        if (!message.attachments.isEmpty()) {
            QJsonArray items = QJsonDocument::fromJson(message.attachments.toUtf8()).array();

            int count = static_cast<int>(items.count());
            notificationText = i18ncp("Number of files attached", "%1 Attachment", "%1 Attachments", count);
            notification->setText(notificationText);

            if (SettingsManager::self()->showAttachments()) {
                QList<QUrl> urls;
                for (const auto &item : items) {
                    const QString local = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
                    const QString folder = QString::number(hash(message.phoneNumberList.toString()));
                    const QString fileName = item.toObject()[SL("fileName")].toString();
                    urls.append(QUrl::fromLocalFile(local + SL("/spacebar/attachments/") + folder + SL("/") + fileName));
                }
                notification->setUrls(urls);
            }
        } else {
            notification->setText(notificationText);
        }
    }

    // copy current pointer to notification, otherwise this would just close the most recent one.
    auto openApp = [notification, message]() {
        notification->close();
        auto *job = new KIO::CommandLauncherJob(SL("spacebar"), {message.phoneNumberList.toString()});
        job->setStartupId(notification->xdgActivationToken().toUtf8());
        job->setDesktopName(SL("org.kde.spacebar"));
        job->start();
    };

    auto defaultAction = notification->addDefaultAction(i18nc("@action open message in application", "Open"));
    connect(defaultAction, &KNotificationAction::activated, this, openApp);

    notification->sendEvent();
}

void ChannelLogger::disableNotificationsForNumber(const QString &numbers)
{
    m_disabledNotificationNumber = PhoneNumberList(numbers);
}

QCoro::Task<bool> ChannelLogger::handleTapbackReaction(Message &message, const QString &reactNumber)
{
    for (const auto &tapback : TAPBACK_REMOVED) {
        if (message.text.startsWith(tapback)) {
            co_return co_await saveTapback(message, reactNumber, tapback, TAPBACK_REMOVED, false, false);
        } else if (message.text == tapback.left(tapback.length() - 1) + SL("an image")) {
            co_return co_await saveTapback(message, reactNumber, tapback, TAPBACK_REMOVED, false, true);
        }
    }

    for (const auto &tapback : TAPBACK_ADDED) {
        if (message.text.startsWith(tapback)) {
            co_return co_await saveTapback(message, reactNumber, tapback, TAPBACK_ADDED, true, false);
        } else if (message.text == tapback.left(tapback.length() - 1) + SL("an image")) {
            co_return co_await saveTapback(message, reactNumber, tapback, TAPBACK_ADDED, true, true);
        }
    }

    co_return false;
}

QCoro::Task<bool> ChannelLogger::saveTapback(Message &message,
                                             const QString &reactNumber,
                                             const QStringView &tapback,
                                             std::span<const QStringView> list,
                                             const bool &isAdd,
                                             const bool &isImage)
{
    const QString searchText = isImage ? SL("") : message.text.mid(tapback.length(), message.text.length() - tapback.length() - 1);
    const auto id = isImage ? co_await m_database.lastMessageWithAttachment(message.phoneNumberList)
                            : co_await m_database.lastMessageWithText(message.phoneNumberList, searchText);

    if (id) {
        Message msg = (co_await m_database.messagesForNumber(message.phoneNumberList, *id)).front();
        QJsonObject reactions = QJsonDocument::fromJson(msg.tapbacks.toUtf8()).object();
        QJsonArray numbers;
        const QJsonValue number = QJsonValue(reactNumber);

        // limits tapbacks to one per message per number
        for (const auto &keyToRemove : TAPBACK_KEYS) {
            if (reactions.contains(keyToRemove)) {
                numbers = reactions[keyToRemove].toArray();

                for (int i = 0; i < numbers.size(); ++i) {
                    if (numbers.at(i) == number) {
                        numbers.removeAt(i);

                        if (numbers.isEmpty()) {
                            reactions.remove(keyToRemove);
                        } else {
                            reactions[keyToRemove] = numbers;
                        }
                    }
                }
            }
        }

        if (isAdd) {
            const int idx = std::find(list.begin(), list.end(), tapback) - list.begin();

            numbers = reactions[TAPBACK_KEYS[idx]].toArray();

            if (!numbers.contains(number)) {
                numbers.append(number);
            }

            reactions.insert(TAPBACK_KEYS[idx], numbers);
        }

        if (reactions.isEmpty()) {
            msg.tapbacks = QString();
        } else {
            QJsonDocument jsonDoc;
            jsonDoc.setObject(reactions);
            msg.tapbacks = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
        }

        m_database.updateMessageTapbacks(*id, msg.tapbacks);

        message.id = msg.id;
        co_return true;
    }

    co_return false;
}

QCoro::Task<void> ChannelLogger::sendTapbackHandler(const QString &numbers, const QString &id, const QString &tapback, const bool &isRemoved)
{
    Message message = (co_await m_database.messagesForNumber(PhoneNumberList(numbers), id)).front();
    const int idx = std::find(TAPBACK_KEYS.cbegin(), TAPBACK_KEYS.cend(), tapback) - TAPBACK_KEYS.cbegin();

    if (message.attachments.isEmpty()) {
        if (isRemoved) {
            message.text = TAPBACK_REMOVED[idx] + message.text + SL("”");
        } else {
            message.text = TAPBACK_ADDED[idx] + message.text + SL("”");
        }
    } else {
        if (isRemoved) {
            message.text = TAPBACK_REMOVED[idx].left(TAPBACK_REMOVED[idx].length() - 1) + SL("an image");
        } else {
            message.text = TAPBACK_ADDED[idx].left(TAPBACK_ADDED[idx].length() - 1) + SL("an image");
        }
    }

    handleTapbackReaction(message, m_ownNumber.toInternational());
    Q_EMIT messageUpdated(numbers, message.id);

    for (const auto &phoneNumber : PhoneNumberList(numbers)) {
        ModemManager::ModemMessaging::Message m;
        m.number = phoneNumber.toE164();
        m.text = message.text;

        auto maybeReply = ModemController::instance().createMessage(m);

        if (!maybeReply) {
            qDebug() << "No modem";
            co_return;
        }

        const QDBusReply<QDBusObjectPath> msgPathResult = *maybeReply;

        if (!msgPathResult.isValid()) {
            co_return;
        }

        ModemManager::Sms::Ptr mmMessage = QSharedPointer<ModemManager::Sms>::create(msgPathResult.value().path());

        QDBusReply<void> sendResult = mmMessage->send();

        if (!sendResult.isValid()) {
            qDebug() << sendResult.error().message();
            co_return;
        }
    }
}

void ChannelLogger::syncSettings()
{
    SettingsManager::self()->load();
}

QCoro::Task<QByteArray> ChannelLogger::uploadMessage(const QByteArray &data)
{
    const QString url = SettingsManager::self()->mmsc();
    if (url.length() < 10) {
        qDebug() << "Invalid URL provided";
        co_return BL("");
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QByteArray response = co_await QtConcurrent::run(&ECurl::networkRequest, &m_curl, url, data);
#else
    const QByteArray response = co_await QtConcurrent::run(&m_curl, &ECurl::networkRequest, url, data);
#endif

    if (response.isNull()) {
        co_return QByteArray();
    } else {
        co_return response;
    }
}

QCoro::Task<void> ChannelLogger::downloadMessage(const MmsMessage message)
{
    const QString url = message.contentLocation;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QByteArray response = co_await QtConcurrent::run(&ECurl::networkRequest, &m_curl, url, BL(""));
#else
    const QByteArray response = co_await QtConcurrent::run(&m_curl, &ECurl::networkRequest, url, BL(""));
#endif

    if (response.isNull()) {
        if (!message.databaseId.isEmpty()) {
            Q_EMIT manualDownloadFinished(message.databaseId, true);
        } else {
            createDownloadNotification(message);
        }
    } else {
        // if message exists, do not create a new download notification
        if (!message.databaseId.isEmpty()) {
            Q_EMIT manualDownloadFinished(message.databaseId, response.isEmpty());
        } else if (response.isEmpty()) {
            createDownloadNotification(message);
        }

        if (!response.isEmpty()) {
            handleDownloadedMessage(response, message.contentLocation, message.expiry);

            if (!message.transactionId.isEmpty()) {
                sendDeliveryAcknowledgement(message.transactionId); // acknowledge download
            }
        }
    }
}
