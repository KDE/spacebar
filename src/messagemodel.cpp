// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QFutureWatcher>
#include <QSharedPointer>

#include <qofonomessagemanager.h>
#include <qofonomanager.h>
#include <qofonomessage.h>

#include <phonenumberutils.h>

#include <global.h>
#include <contactphonenumbermapper.h>

#include "asyncdatabase.h"
#include "channelhandler.h"


MessageModel::MessageModel(ChannelHandler &handler, const QString &phoneNumber, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_phoneNumber(phoneNumber)
    , m_personData(new KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(phoneNumber), this))
{
    connect(&m_handler.msgManager(), &QOfonoMessageManager::incomingMessage, this, [=](const QString &text, const QVariantMap &info) {
        if (PhoneNumberUtils::normalize(info[SL("Sender")].toString()) != m_phoneNumber) {
            return; // Message is not for this model
        }
        Message message;
        // message.id = m_database->lastId() + 1; FIXME, we don't know the id so this message will not be marked as read.
        message.read = false;
        message.text = text;
        message.datetime = QDateTime::fromString(info[SL("SentTime")].toString().split(QChar(u'+'))[0], Qt::ISODate);
        message.sentByMe = false;
        message.deliveryStatus = MessageState::Received; // If it arrived here, it was
        message.phoneNumber = info[SL("Sender")].toString();

        qDebug() << "Received from id" << message.phoneNumber;
        addMessage(message);
    });

    connect(&m_handler.database(), &AsyncDatabase::messagesFetchedForNumber,
            this, [this](const QString &phoneNumber, const QVector<Message> &messages) {
        if (phoneNumber == m_phoneNumber) {
            beginResetModel();
            m_messages = messages;
            endResetModel();
        }
    });

    Q_EMIT m_handler.database().requestMessagesForNumber(m_phoneNumber);
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
        {Role::IdRole, BL("id")}
    };
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.count()) {
        return false;
    }

    switch (role) {
    case Role::TextRole:
        return m_messages.at(index.row()).text;
    case Role::TimeRole:
        return m_messages.at(index.row()).datetime.time();
    case Role::DateRole:
        return m_messages.at(index.row()).datetime.date();
    case Role::SentByMeRole:
        return m_messages.at(index.row()).sentByMe;
    case Role::ReadRole:
        return m_messages.at(index.row()).read;
    case Role::DeliveryStateRole:
        return DeliveryState(m_messages.at(index.row()).deliveryStatus);
    case Role::IdRole:
        return m_messages.at(index.row()).id;
    }

    return {};
}

int MessageModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : m_messages.count();
}

KPeople::PersonData *MessageModel::person() const
{
    return m_personData;
}

QString MessageModel::phoneNumber() const
{
    return m_phoneNumber;
}

void MessageModel::addMessage(const Message &message)
{
    beginInsertRows({}, 0, 0);
    m_messages.prepend(message);
    endInsertRows();
}

void MessageModel::sendMessage(const QString &text)
{
    QString intermediateId = Database::generateRandomId();

    Message message;
    message.id = intermediateId;
    message.phoneNumber = m_phoneNumber;
    message.text = text;
    message.datetime = QDateTime::currentDateTime(); // NOTE: there is also tpMessage.sent(), doesn't seem to return a proper time, but maybe a backend bug?
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.deliveryStatus = MessageState::Unknown; // if this signal is called, the message was delivered.

    // Add message to model
    addMessage(message);

    std::shared_ptr<QFutureWatcher<std::pair<bool, QString>>> watcher(
                new QFutureWatcher<std::pair<bool, QString>>(),
                [](auto *watcher) {
        watcher->deleteLater();
    });
    connect(watcher.get(), &QFutureWatcherBase::finished, this, [=] {
        const auto result = watcher->result();
        bool success = result.first;

        if (success) {
            const QString &path = result.second;

            const auto modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
                return message.id == intermediateId;
            });

            if (modelIt != m_messages.cend()) {
                modelIt->id = path;

                const int i = std::distance(m_messages.begin(), modelIt);

                Q_EMIT m_handler.database().requestAddMessage(*modelIt);

                const auto ofonoMessage = std::make_shared<QOfonoMessage>();
                ofonoMessage->setMessagePath(path);

                // Message can already be sent and deleted here, should only happpen with phonesim
                // Assume message was sent
                if (!ofonoMessage->isValid()) {
                    qWarning() << "Failed to track message state, as it was already deleted";
                    modelIt->deliveryStatus = MessageState::Sent;
                    Q_EMIT m_handler.database().requestUpdateMessageDeliveryState(path, MessageState::Sent);
                    Q_EMIT dataChanged(index(i), index(i), {Role::DeliveryStateRole});
                }

                connect(ofonoMessage.get(), &QOfonoMessage::stateChanged, this, [=] {
                    MessageState state = parseMessageState(ofonoMessage->state());
                    modelIt->deliveryStatus = state;
                    Q_EMIT m_handler.database().requestUpdateMessageDeliveryState(path, state);
                    Q_EMIT dataChanged(index(i), index(i), {Role::DeliveryStateRole});
                });
            } else {
                qWarning() << "Failed to find message that was just sent. This is a bug";
            }
        }
    });

    watcher->setFuture(m_handler.msgManager().sendMessage(m_phoneNumber, text));
}

void MessageModel::markMessageRead(const int id)
{
    Q_EMIT m_handler.database().requestMarkMessageRead(id);
}
