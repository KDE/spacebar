// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"
#include "channelhandler.h"
#include "utils.h"

#include <QJsonObject>
#include <QMimeDatabase>
#include <QMimeType>

#include <contactphonenumbermapper.h>
#include <database.h>
#include <global.h>
#include <phonenumberlist.h>

#include <QCoroFuture>

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

    connect(m_handler.interface(), &OrgKdeSpacebarDaemonInterface::messageAdded, this, &MessageModel::messageAdded);

    connect(m_handler.interface(), &OrgKdeSpacebarDaemonInterface::messageUpdated, this, &MessageModel::messageUpdated);

    connect(m_handler.interface(), &OrgKdeSpacebarDaemonInterface::manualDownloadFinished, this, [this](const QString &id, const bool isEmpty) {
        if (isEmpty) {
            updateMessageState(id, MessageState::Failed, true);
        } else {
            const auto idx = getMessageIndex(id);
            deleteMessage(id, idx.second, QStringList());
        }
    });

    fetchMessages(QString());
}

QCoro::Task<void> MessageModel::fetchMessages(const QString &id, const int limit)
{
    const auto messages = co_await m_handler.database().messagesForNumber(m_phoneNumberList, id, limit);

    if (limit == -1) {
        beginInsertRows({}, 0, messages.size() - 1);
        for (auto &&message : messages) {
            m_messages.push_back(message);
        }
        endInsertRows();
    } else {
        if (messages.size() == 1) {
            beginInsertRows({}, m_messages.size(), m_messages.size());
            m_messages.insert(m_messages.begin(), messages.at(0));
            endInsertRows();
        } else {
            beginResetModel();
            m_messages = messages;
            endResetModel();
        }
    }

    Q_EMIT messagesFetched();
}

QCoro::Task<void> MessageModel::fetchUpdatedMessage(const QString &id)
{
    const auto &[message, i] = getMessageIndex(id);
    const auto messages = co_await m_handler.database().messagesForNumber(m_phoneNumberList, id);

    if (!messages.empty()) {
        message->text = messages.front().text;
        message->datetime = messages.front().datetime;
        message->deliveryStatus = messages.front().deliveryStatus;
        message->attachments = messages.front().attachments;
        message->smil = messages.front().smil;
        message->deliveryReport = messages.front().deliveryReport;
        message->readReport = messages.front().readReport;
        message->tapbacks = messages.front().tapbacks;

        Q_EMIT dataChanged(index(i), index(i));
    }
}

void MessageModel::fetchAllMessages()
{
    fetchMessages(QString(), -1);
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {{Role::TextRole, BL("text")},
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
            {Role::SizeRole, BL("size")},
            {Role::TapbacksRole, BL("tapbacks")}};
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || size_t(index.row()) >= m_messages.size()) {
        return false;
    }

    // message order is reversed from the C++ side instead of in QML since sectioning doesn't work right otherwise
    Message message = m_messages.at((m_messages.size() - 1) - index.row());
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
    case Role::TapbacksRole:
        return message.tapbacks;
    }

    return {};
}

int MessageModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : m_messages.size();
}

QVector<Person> MessageModel::people() const
{
    return m_peopleData;
}

PhoneNumberList MessageModel::phoneNumberList() const
{
    return m_phoneNumberList;
}

QString MessageModel::sendingNumber() const
{
    QString number = m_handler.interface()->ownNumber();
    if (number == SL("The name org.kde.Spacebar was not provided by any .service files")) {
        return QString();
    } else {
        return PhoneNumber(number).toInternational();
    }
}

QString MessageModel::attachmentsFolder() const
{
    const QString folder = QString::number(hash(m_phoneNumberList.toString()));
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

    QJsonObject object{{SL("filePath"), path.toString()},
                       {SL("name"), path.fileName()},
                       {SL("fileName"), fileName},
                       {SL("size"), data.length()},
                       {SL("mimeType"), mime.name()},
                       {SL("iconName"), mime.iconName()}};

    return object;
}

void MessageModel::messageAdded(const QString &numbers, const QString &id)
{
    if (PhoneNumberList(numbers) != m_phoneNumberList) {
        return; // Message is not for this model
    }

    fetchMessages(id);
}

void MessageModel::messageUpdated(const QString &numbers, const QString &id)
{
    if (PhoneNumberList(numbers) != m_phoneNumberList) {
        return; // Message is not for this model
    }

    fetchUpdatedMessage(id);
}

void MessageModel::sendTapback(const QString &id, const QString &tapback, const bool &isRemoved)
{
    m_handler.interface()->sendTapback(m_phoneNumberList.toString(), id, tapback, isRemoved);
}

void MessageModel::sendMessage(const QString &text, const QStringList &files, const qint64 &totalSize)
{
    Message message;
    message.id = Database::generateRandomId();
    message.phoneNumberList = m_phoneNumberList;
    message.text = text;
    message.sentByMe = true;
    message.deliveryStatus = MessageState::Pending;

    beginInsertRows({}, m_messages.size(), m_messages.size());
    m_messages.insert(m_messages.begin(), message);
    endInsertRows();

    m_handler.interface()->sendMessage(m_phoneNumberList.toString(), message.id, text, files, totalSize);

    // update chat list
    Q_EMIT m_handler.messagesChanged(m_phoneNumberList);
}

QPair<Message *, int> MessageModel::getMessageIndex(const QString &id)
{
    auto modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
        return message.id == id;
    });

    Q_ASSERT(modelIt != m_messages.cend());

    const size_t i = (m_messages.size() - 1) - std::distance(m_messages.begin(), modelIt);
    return qMakePair(&(*modelIt.base()), i);
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
    m_messages.erase(m_messages.begin() + (m_messages.size() - index - 1));
    endRemoveRows();

    // update chat list only if it was the most recent message
    if (size_t(index) == m_messages.size()) {
        Q_EMIT m_handler.messagesChanged(m_phoneNumberList);
    }
}

void MessageModel::saveAttachments(const QStringList &attachments)
{
    const QString sourceFolder = attachmentsFolder();
    const QString targetFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    for (const auto &i : attachments) {
        QFile::copy(sourceFolder + QStringLiteral("/") + i, targetFolder + QStringLiteral("/") + i);
    }
}

void MessageModel::disableNotifications(const PhoneNumberList &phoneNumberList)
{
    m_handler.interface()->disableNotificationsForNumber(phoneNumberList.toString().isNull() ? SL("") : phoneNumberList.toString());
}
