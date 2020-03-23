#include "messagemodel.h"

#include <QDebug>
#include <QtQml>

#include <TelepathyQt/PendingReady>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Message>

#include "global.h"

MessageModel::MessageModel(Database *database, const QString &phoneNumber, Tp::TextChannelPtr channel, const QString &personUri, QObject *parent)
    : QAbstractListModel(parent)
{
    m_database = database;
    m_phoneNumber = phoneNumber;
    m_personData = new KPeople::PersonData(personUri, this);

    m_channel = channel;

    connect(channel->becomeReady(), &Tp::PendingReady::finished, this, [](Tp::PendingOperation  *op) {
        if (op->isError()) {
            qDebug() << op->errorMessage();
            return;
        }
    });
    m_messages = m_database->messagesForNumber(m_phoneNumber);
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {Role::TextRole, BL("text")},
        {Role::TimeRole, BL("time")},
        {Role::SentByMeRole, BL("sentByMe")},
        {Role::ReadRole, BL("read")}
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
        return m_messages.at(index.row()).time;
    case Role::SentByMeRole:
        return m_messages.at(index.row()).sentByMe;
    case Role::ReadRole:
        return m_messages.at(index.row()).read;
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
    beginInsertRows({}, m_messages.count(), m_messages.count());
    m_messages.append(message);
    endInsertRows();
    m_database->addMessage(message);
}

void MessageModel::sendMessage(const QString &text)
{
    auto *op = m_channel->send(text);
    connect(op, &Tp::PendingOperation::finished, this, [=]() {
        qDebug() << "Message sent";
        auto tpMessage = op->message();

        Message message;
        message.phoneNumber = m_phoneNumber;
        message.text = tpMessage.text();
        message.time = tpMessage.sent();
        message.read = true; // Messages sent by us are automatically read.
        message.sentByMe = true; // only called if message sent by us.
        message.delivered = true; // if this signal is called, the message was delivered.

        addMessage(message);
    });
}
