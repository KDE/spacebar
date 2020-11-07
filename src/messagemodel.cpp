// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Message>

#include <global.h>
#include "asyncdatabase.h"

MessageModel::MessageModel(AsyncDatabase *database, const QString &phoneNumber, const Tp::TextChannelPtr &channel, const QString &personUri, QObject *parent)
    : QAbstractListModel(parent)
    , m_database(database)
    , m_channel(channel)
    , m_phoneNumber(phoneNumber)
    , m_personData(new KPeople::PersonData(personUri, this))
{
    connect(channel.data(), &Tp::TextChannel::messageReceived, this, [=](const Tp::ReceivedMessage &receivedMessage){
        if (receivedMessage.isDeliveryReport()) {
            qDebug() << "received delivery report";
            // TODO: figure out correct ID and mark it as delivered.
            return;
        }
        Message message;
        // message.id = m_database->lastId() + 1; FIXME, we don't know the id so this message will not be marked as read.
        message.read = false;
        message.text = receivedMessage.text();
        message.datetime = receivedMessage.received();
        message.sentByMe = false;
        message.delivered = true; // If it arrived here, it was
        message.phoneNumber = receivedMessage.sender()->id();
        qDebug() << "Received from id" << receivedMessage.sender()->id();
        addMessage(message);
    });

    connect(m_channel->becomeReady(), &Tp::PendingReady::finished, this, [=](Tp::PendingOperation *op) {
        if (op->isError()) {
            qDebug() << "channel not ready" << op->errorMessage();
            return;
        }

        m_isReady = true;
        qDebug() << "channel is now officially ready";
        emit isReadyChanged();
    });

    connect(m_database, &AsyncDatabase::messagesFetchedForNumber,
            this, [this](const QString &phoneNumber, const QVector<Message> &messages) {
        if (phoneNumber == m_phoneNumber) {
            qDebug() << "Hello messages";
            beginResetModel();
            m_messages = messages;
            endResetModel();
        }
    });

    Q_EMIT m_database->requestMessagesForNumber(m_phoneNumber);
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {Role::TextRole, BL("text")},
        {Role::TimeRole, BL("time")},
        {Role::DateRole, BL("date")},
        {Role::SentByMeRole, BL("sentByMe")},
        {Role::ReadRole, BL("read")},
        {Role::DeliveredRole, BL("delivered")},
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
    case Role::DeliveredRole:
        return m_messages.at(index.row()).delivered;
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
    auto *op = m_channel->send(text, Tp::ChannelTextMessageTypeNormal, {});

    connect(m_database, &AsyncDatabase::lastIdFetched, this, [=](const int lastId) {
        Message message;
        message.id = lastId;
        message.phoneNumber = m_phoneNumber;
        message.text = text;
        message.datetime = QDateTime::currentDateTime(); // NOTE: there is also tpMessage.sent(), doesn't seem to return a proper time, but maybe a backend bug?
        message.read = true; // Messages sent by us are automatically read.
        message.sentByMe = true; // only called if message sent by us.
        message.delivered = false; // if this signal is called, the message was delivered.

        // Add message to model
        qDebug() << "Adding message to model";
        addMessage(message);

        connect(op, &Tp::PendingOperation::finished, this, [=]() {
            qDebug() << "Message sent";
            //auto tpMessage = op->message(); // NOTE: This exists. We don't need it right now though.
            Q_EMIT m_database->requestMarkMessageDelivered(message.id); // TODO DAEMON

            const auto sentMessageIt = std::find_if(m_messages.begin(), m_messages.end(), [&message](const Message &chatMessage) {
                return chatMessage.id == message.id;
            });

            // every sent message should be in the history
            Q_ASSERT(sentMessageIt != m_messages.end());

            sentMessageIt->delivered = true;

            auto modelIndex = index(std::distance(m_messages.begin(), sentMessageIt));

            // to check the distance
            Q_ASSERT(modelIndex.data(MessageModel::TextRole).toString() == sentMessageIt->text);

            disconnect(op, &Tp::PendingOperation::finished, this, nullptr);
            emit dataChanged(modelIndex, modelIndex, {Role::DeliveredRole});
        });
        disconnect(m_database, &AsyncDatabase::lastIdFetched, this, nullptr);
    });

    Q_EMIT m_database->requestLastId();
}

void MessageModel::markMessageRead(const int id)
{
    Q_EMIT m_database->requestMarkMessageRead(id);  // TODO DAEMON
}

bool MessageModel::isReady() const
{
    return m_isReady;
}
