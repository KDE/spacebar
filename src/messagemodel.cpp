// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QtQml>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Message>

#include "global.h"

MessageModel::MessageModel(Database *database, const QString &phoneNumber, const Tp::TextChannelPtr &channel, const QString &personUri, QObject *parent)
    : QAbstractListModel(parent)
{
    m_database = database;
    m_phoneNumber = phoneNumber;
    m_personData = new KPeople::PersonData(personUri, this);

    m_channel = channel;

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
    m_messages = m_database->messagesForNumber(m_phoneNumber);
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
    Message message;
    message.id = m_database->lastId() + 1;
    message.phoneNumber = m_phoneNumber;
    message.text = text;
    message.datetime = QDateTime::currentDateTime(); // NOTE: there is also tpMessage.sent(), doesn't seem to return a proper time, but maybe a backend bug?
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.delivered = false; // if this signal is called, the message was delivered.

    // Add message to model
    addMessage(message);

    // Store message in database
    //m_database->addMessage(message);

    connect(op, &Tp::PendingOperation::finished, this, [=]() {
        qDebug() << "Message sent";
        //auto tpMessage = op->message(); // NOTE: This exists. We don't need it right now though.
        m_database->markMessageDelivered(message.id); // TODO DAEMON
        for (int i = 0; i < m_messages.size(); i++) {
            if (m_messages.at(i).id == message.id) {
                m_messages[i].delivered = true;
                emit dataChanged(index(i), index(i), {Role::DeliveredRole});
            };
        }
    });
}

void MessageModel::markMessageRead(const int id)
{
    m_database->markMessageRead(id);  // TODO DAEMON
}

bool MessageModel::isReady() const
{
    return m_isReady;
}
