// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>

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
        message.datetime = info[SL("SentTime")].toDateTime();
        message.sentByMe = false;
        message.delivered = true; // If it arrived here, it was
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
    QString intermediateId = Database::generateRandomId();

    Message message;
    message.id = intermediateId;
    message.phoneNumber = m_phoneNumber;
    message.text = text;
    message.datetime = QDateTime::currentDateTime(); // NOTE: there is also tpMessage.sent(), doesn't seem to return a proper time, but maybe a backend bug?
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.delivered = false; // if this signal is called, the message was delivered.

    // Add message to model
    addMessage(message);

    connect(&m_handler.msgManager(), &QOfonoMessageManager::sendMessageComplete, this, [=](bool success, const QString& path) {
        if (success) {
            const auto modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
                return message.id == intermediateId;
            });

            if (modelIt != m_messages.cend()) {
                modelIt->id = path;
                modelIt->delivered = true;

                const int i = std::distance(m_messages.begin(), modelIt);

                dataChanged(index(i), index(i), {Role::DeliveredRole});

                m_handler.database().requestAddMessage(*modelIt);
            } else {
                qWarning() << "Failed to find message that was just sent. This is a bug";
            }
            disconnect(&m_handler.msgManager(), &QOfonoMessageManager::sendMessageComplete, nullptr, nullptr);
        }
    });
    m_handler.msgManager().sendMessage(m_phoneNumber, text);
}

void MessageModel::markMessageRead(const int id)
{
    Q_EMIT m_handler.database().requestMarkMessageRead(id);
}
