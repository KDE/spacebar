// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QMimeType>

#include <KLocalizedString>

#include <ModemManagerQt/Sms>

#include <global.h>
#include <contactphonenumbermapper.h>
#include <phonenumberlist.h>

#include "asyncdatabase.h"
#include "channelhandler.h"
#include "utils.h"
#include "modemcontroller.h"
#include "settingsmanager.h"

#include <QCoroDBusPendingReply>

MessageModel::MessageModel(ChannelHandler &handler, const PhoneNumberList &phoneNumberList, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_phoneNumberList(phoneNumberList)
{
    disableNotifications(m_phoneNumberList);

    for (const auto &number : phoneNumberList) {
        Person person;
        person.m_name = KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(number)).name();
        person.m_phoneNumber = number.toInternational();
        m_peopleData.append(person);
    }

    connect(m_handler.interface(), &OrgKdeSpacebarDaemonInterface::messageAdded, this, &MessageModel::messagedAdded);

    connect(m_handler.interface(), &OrgKdeSpacebarDaemonInterface::manualDownloadFinished, this, [this](const QString &id, const bool isEmpty) {
        if (isEmpty) {
            updateMessageState(id, MessageState::Failed, true);
        } else {
            const auto idx = getMessageIndex(id);
            deleteMessage(id, idx.second, QStringList());
        }
    });

    connectFuture(m_handler.database().messagesForNumber(m_phoneNumberList, QString()), this, &MessageModel::updateModel);
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {Role::TextRole, BL("text")},
        {Role::TimeRole, BL("time")},
        {Role::DateRole, BL("date")},
        {Role::SentByMeRole, BL("sentByMe")},
        {Role::ReadRole, BL("read")},
        {Role::DeliveryStateRole, BL("deliveryState")},
        {Role::IdRole, BL("id")},
        {Role::AttachmentsRole, BL("attachments")},
        {Role::SmilRole, BL("smil")},
        {Role::FromNumberRole, BL("fromNumber")},
        {Role::MessageIdRole, BL("messageId")},
        {Role::DeliveryReportRole, BL("deliveryReport")},
        {Role::ReadReportRole, BL("readReport")},
        {Role::PendingDownloadRole, BL("pendingDownload")},
        {Role::ContentLocationRole, BL("contentLocation")},
        {Role::ExpiresRole, BL("expires")},
        {Role::ExpiresDateTimeRole, BL("expiresDateTime")},
        {Role::SizeRole, BL("size")}
    };
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.count()) {
        return false;
    }

    // message order is reversed from the C++ side instead of in QML since sectioning doesn't work right otherwise
    Message message = m_messages.at((m_messages.count() - 1) - index.row());
    switch (role) {
    case Role::TextRole:
        return message.text;
    case Role::TimeRole:
        return message.datetime.time();
    case Role::DateRole:
        return message.datetime.date();
    case Role::SentByMeRole:
        return message.sentByMe;
    case Role::ReadRole:
        return message.read;
    case Role::DeliveryStateRole:
        return DeliveryState(message.deliveryStatus);
    case Role::IdRole:
        return message.id;
    case Role::AttachmentsRole:
        return message.attachments;
    case Role::SmilRole:
        return message.smil;
    case Role::FromNumberRole:
        return message.fromNumber;
    case Role::MessageIdRole:
        return message.messageId;
    case Role::DeliveryReportRole:
        return message.deliveryReport;
    case Role::ReadReportRole:
        return message.readReport;
    case Role::PendingDownloadRole:
        return message.pendingDownload;
    case Role::ContentLocationRole:
        return message.contentLocation;
    case Role::ExpiresRole:
        return message.expires;
    case Role::ExpiresDateTimeRole:
        return message.expires.toLocalTime().toString(SL("MMM d, h:mm ap"));
    case Role::SizeRole:
        return message.size;
    }

    return {};
}

int MessageModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : m_messages.count();
}

QVector<Person> MessageModel::people() const
{
    return m_peopleData;
}

PhoneNumberList MessageModel::phoneNumberList() const
{
    return m_phoneNumberList;
}

QString MessageModel::attachmentsFolder() const
{
    const QString folder = QString::number(qHash(m_phoneNumberList.toString()));
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments/") + folder;
}

QVariant MessageModel::fileInfo(const QUrl &path)
{
    QFile file(path.toLocalFile());
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForData(data);
    QString fileName = Database::generateRandomId() + SL(".") + mime.preferredSuffix();

    QJsonObject object
    {
        { SL("filePath"), path.toString() },
        { SL("name"), path.fileName() },
        { SL("fileName"), fileName },
        { SL("size"), data.length() },
        { SL("mimeType"), mime.name() },
        { SL("iconName"), mime.iconName() }
    };

    return object;
}

void MessageModel::messagedAdded(const QString &numbers, const QString &id)
{
    if (PhoneNumberList(numbers) != m_phoneNumberList) {
        return; // Message is not for this model
    }

    connectFuture(m_handler.database().messagesForNumber(m_phoneNumberList, id), this, &MessageModel::updateModel);
}

void MessageModel::updateModel(const QVector<Message> &messages)
{
    if (messages.count() == 1) {
        beginInsertRows({}, m_messages.count(), m_messages.count());
        m_messages.prepend(messages.at(0));
        endInsertRows();
    } else {
        beginResetModel();
        m_messages = messages;
        endResetModel();
    }
}

void MessageModel::addMessage(const Message &message)
{
    beginInsertRows({}, m_messages.count(), m_messages.count());
    m_messages.prepend(message);
    endInsertRows();

    // save to database
    m_handler.database().addMessage(message);
}

void MessageModel::sendMessage(const QString &text, const QStringList &files, const long totalSize)
{
    [this, text, files, totalSize] () -> QCoro::Task<void> {
        QString result;
        // check if it is a mms message
        if (m_phoneNumberList.size() > 1 || files.length() > 0) {
            if (SettingsManager::self()->groupConversation()) {
                sendMessageInternalMms(m_phoneNumberList, text, files, totalSize);
            } else {
                // send as individual messages
                for (const auto &phoneNumber : m_phoneNumberList) {
                    if (files.length() > 0) {
                        sendMessageInternalMms(PhoneNumberList(phoneNumber.toInternational()), text, files, totalSize);
                    } else {
                        result = co_await sendMessageInternal(phoneNumber, text);
                    }
                }
            }
        } else {
            result = co_await sendMessageInternal(m_phoneNumberList.first(), text);
        }

        if (result.isEmpty()) {
            qDebug() << "Message sent successfully";
        } else {
            qDebug() << "Failed successfully" << result;
        }
    }();
}

QPair<Message *, int> MessageModel::getMessageIndex(const QString &id)
{
    auto modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
        return message.id == id;
    });

    Q_ASSERT(modelIt != m_messages.cend());

    const int i = (m_messages.count() - 1) - std::distance(m_messages.begin(), modelIt);
    return qMakePair(modelIt, i);
}

void MessageModel::updateMessageState(const QString &id, MessageState state, const bool temp)
{
    const auto &[message, i] = getMessageIndex(id);

    message->deliveryStatus = state;

    if (!temp) {
        m_handler.database().updateMessageDeliveryState(id, state);
    }

    Q_EMIT dataChanged(index(i), index(i), {Role::DeliveryStateRole});
}

QCoro::Task<QString> MessageModel::sendMessageInternal(const PhoneNumber &phoneNumber, const QString &text)
{
    ModemManager::ModemMessaging::Message m;
    m.number = phoneNumber.toE164();
    m.text = text;//Utils::textToHtml(text);

    Message message;
    message.id = Database::generateRandomId();
    message.phoneNumberList = PhoneNumberList(phoneNumber.toInternational());
    message.text = text;//Utils::textToHtml(text);
    message.datetime = QDateTime::currentDateTime();
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.deliveryStatus = MessageState::Pending; // this signal is changed to either failed or sent, depending if the message was delivered.


    auto maybeReply = ModemController::instance().createMessage(m);

    if (!maybeReply) {
        message.deliveryStatus = MessageState::Failed;
        addMessage(message);
        co_return QStringLiteral("No modem");
    }

    const QDBusReply<QDBusObjectPath> msgPathResult = co_await *maybeReply;

    if (!msgPathResult.isValid()) {
        message.deliveryStatus = MessageState::Failed;
        addMessage(message);
        co_return msgPathResult.error().message();
    }

    // Add message to model
    addMessage(message);

    ModemManager::Sms::Ptr mmMessage = QSharedPointer<ModemManager::Sms>::create(msgPathResult.value().path());

    connect(mmMessage.get(), &ModemManager::Sms::stateChanged, this, [mmMessage, message, this] {
        qDebug() << "state changed" << mmMessage->state();

        switch (mmMessage->state()) {
            case MM_SMS_STATE_SENT:
                updateMessageState(message.id, MessageState::Sent);
                break;
            case MM_SMS_STATE_RECEIVED:
                // Should not happen
                qWarning() << "Received a message we sent";
                break;
            case MM_SMS_STATE_RECEIVING:
                // Should not happen
                qWarning() << "Receiving a message we sent";
                break;
            case MM_SMS_STATE_SENDING:
                updateMessageState(message.id, MessageState::Pending);
                break;
            case MM_SMS_STATE_STORED:
                updateMessageState(message.id, MessageState::Pending);
                break;
            case MM_SMS_STATE_UNKNOWN:
                updateMessageState(message.id, MessageState::Unknown);
                break;
        }
    });

    connect(mmMessage.get(), &ModemManager::Sms::deliveryStateChanged, this, [=] {
        MMSmsDeliveryState state = mmMessage->deliveryState();
        // TODO does this even change?
        // TODO do something with the state
        qDebug() << "deliverystate changed" << state;
    });

    QDBusReply<void> sendResult = co_await mmMessage->send();

    if (!sendResult.isValid()) {
        updateMessageState(message.id, MessageState::Failed);
        co_return sendResult.error().message();
    }

    co_return QString();
}

QCoro::Task<QString> MessageModel::sendMessageInternalMms(const PhoneNumberList &phoneNumberList, const QString &text, const QStringList &files, const long totalSize)
{
    Message message;
    message.phoneNumberList = phoneNumberList;
    message.text = text;//Utils::textToHtml(text);
    message.datetime = QDateTime::currentDateTime();
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.deliveryStatus = MessageState::Pending; // if this signal is called, the message was delivered.

    QString to = phoneNumberList.toString();
    const QStringList toList = to.replace(SL("-"), QString()).replace(SL(" "), QString()).split(SL(";"));
    QString from = Utils::instance()->ownNumber();

    MmsMessage mmsMessage;
    mmsMessage.ownNumber = PhoneNumber(from);
    mmsMessage.from = from.replace(SL("-"), QString()).replace(SL(" "), QString());
    mmsMessage.to = toList;
    mmsMessage.text = message.text;
    QByteArray data;
    m_handler.mms().encodeMessage(mmsMessage, data, files, totalSize);

    // update message with encoded content parts
    message.id = Database::generateRandomId();
    message.text = mmsMessage.text;
    message.attachments = mmsMessage.attachments;
    message.smil = mmsMessage.smil;

    // Add message to model
    addMessage(message);

    //send message
    connect(&m_handler.mms(), &Mms::uploadFinished, this, [this, message](const QByteArray &response) {
        disconnect(&m_handler.mms(), &Mms::uploadError, nullptr, nullptr);
        disconnect(&m_handler.mms(), &Mms::uploadFinished, nullptr, nullptr);
        if (response.length() == 0) {
            updateMessageState(message.id, MessageState::Failed);
        } else {
            MmsMessage mmsMessage;
            m_handler.mms().decodeConfirmation(mmsMessage, response);
            if (mmsMessage.responseStatus == 0) {
                updateMessageState(message.id, MessageState::Sent);

                if (!mmsMessage.messageId.isEmpty()) {
                    m_handler.database().updateMessageSent(message.id, mmsMessage.messageId, mmsMessage.contentLocation);
                }
            } else {
                updateMessageState(message.id, MessageState::Failed);
                qDebug() << mmsMessage.responseText;
            }
        }
    });
    connect(&m_handler.mms(), &Mms::uploadError, this, [this, message]() {
        disconnect(&m_handler.mms(), &Mms::uploadError, nullptr, nullptr);
        disconnect(&m_handler.mms(), &Mms::uploadFinished, nullptr, nullptr);
        updateMessageState(message.id, MessageState::Failed);
    });
    m_handler.mms().uploadMessage(data);

    co_return QString();
}

void MessageModel::markMessageRead(const int id)
{
    m_handler.database().markMessageRead(id);
}

void MessageModel::downloadMessage(const QString &id, const QString &url, const QDateTime &expires)
{
    updateMessageState(id, MessageState::Pending, true);
    m_handler.interface()->manualDownload(id, url, expires);
}

void MessageModel::deleteMessage(const QString &id, const int index, const QStringList &files)
{
    m_handler.database().deleteMessage(id);

    // delete attachments
    const QString sourceFolder = attachmentsFolder();
    for (const auto &file : files) {
        if (!file.isEmpty()) {
            QFile::remove(sourceFolder + SL("/") + file);
        }
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_messages.remove(m_messages.count() - index - 1);
    endRemoveRows();
}

void MessageModel::saveAttachments(const QStringList &attachments)
{
    const QString sourceFolder = attachmentsFolder();
    const QString targetFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    for (const auto& i : attachments) {
        QFile::copy(sourceFolder + QStringLiteral("/") + i, targetFolder + QStringLiteral("/") + i);
    }
}

void MessageModel::disableNotifications(const PhoneNumberList &phoneNumberList)
{
    m_handler.interface()->disableNotificationsForNumber(phoneNumberList.toString());
}
