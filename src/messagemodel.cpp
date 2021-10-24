// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QFutureWatcher>

#include <KTextToHTML>
#include <KLocalizedString>

#include <ModemManagerQt/Sms>

#include <phonenumberutils.h>

#include <global.h>
#include <contactphonenumbermapper.h>

#include "asyncdatabase.h"
#include "channelhandler.h"
#include "utils.h"
#include "modemcontroller.h"

#include <QCoroDBusPendingReply>

MessageModel::MessageModel(ChannelHandler &handler, const QString &phoneNumber, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_phoneNumber(phoneNumberUtils::normalizeNumber(phoneNumber))
    , m_personData(new KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(phoneNumber), this))
{
    disableNotifications(m_phoneNumber);

    connect(&ModemController::instance(), &ModemController::messageAdded, this, [this](ModemManager::Sms::Ptr msg, bool received) {
        if (!received) {
            return;
        }

        if (phoneNumberUtils::normalizeNumber(msg->number()) != m_phoneNumber) {
            return; // Message is not for this model
        }
        Message message;
        message.id = Database::generateRandomId();
        message.read = false;
        message.text = Utils::textToHtml(msg->text());
        message.datetime = msg->timestamp();
        message.sentByMe = false;
        message.deliveryStatus = MessageState::Received; // If it arrived here, it was
        message.phoneNumber = msg->number();

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
    m_messages.prepend(message);
    endInsertRows();
}

void MessageModel::sendMessage(const QString &text)
{
    [this, text] () -> QCoro::Task<void> {
        const QString result = co_await sendMessageInternal(text);

        if (result.isEmpty()) {
            qDebug() << "Message sent successfully";
        } else {
            qDebug() << "Failed successfully" << result;
        }
    }();
}

QPair<Message *, int> MessageModel::getMessageIndex(const QString &path)
{
    Message *modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
        return message.id == path;
    });

    Q_ASSERT(modelIt != m_messages.cend());

    const int i = (m_messages.count() - 1) - std::distance(m_messages.begin(), modelIt);
    return qMakePair(modelIt, i);
}

void MessageModel::updateMessageState(const QString &msgPath, MessageState state)
{
    const auto idx = getMessageIndex(msgPath);

    idx.first->deliveryStatus = state;
    Q_EMIT m_handler.database().requestUpdateMessageDeliveryState(msgPath, state);
    Q_EMIT dataChanged(index(idx.second), index(idx.second), {Role::DeliveryStateRole});
}

QCoro::Task<QString> MessageModel::sendMessageInternal(const QString &text)
{
    ModemManager::ModemMessaging::Message m;
    m.number = phoneNumberUtils::normalizeNumber(m_phoneNumber, phoneNumberUtils::PhoneNumberFormat::E164);
    m.text = text;//Utils::textToHtml(text);

    Message message;
    message.phoneNumber = m_phoneNumber;
    message.text = text;//Utils::textToHtml(text);
    message.datetime = QDateTime::currentDateTime();
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.deliveryStatus = MessageState::Unknown; // if this signal is called, the message was delivered.


    auto maybeReply = ModemController::instance().createMessage(m);

    if (!maybeReply) {
        co_return QStringLiteral("No modem");
    }

    const QDBusReply<QDBusObjectPath> msgPathResult = co_await *maybeReply;

    if (!msgPathResult.isValid()) {
        co_return msgPathResult.error().message();
    }

    const QString msgPath = msgPathResult.value().path();

    message.id = msgPath;
    // Add message to model
    addMessage(message);

    ModemManager::Sms::Ptr mmMessage = QSharedPointer<ModemManager::Sms>::create(msgPath);

    connect(mmMessage.get(), &ModemManager::Sms::stateChanged, this, [mmMessage, msgPath, this] {
        qDebug() << "state changed" << mmMessage->state();

        switch (mmMessage->state()) {
            case MM_SMS_STATE_SENT:
                updateMessageState(msgPath, MessageState::Sent);
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
                updateMessageState(msgPath, MessageState::Pending);
                break;
            case MM_SMS_STATE_STORED:
                updateMessageState(msgPath, MessageState::Pending);
                break;
            case MM_SMS_STATE_UNKNOWN:
                updateMessageState(msgPath, MessageState::Unknown);
                break;
        }
    });

    connect(mmMessage.get(), &ModemManager::Sms::deliveryStateChanged, this, [=, this] {
        MMSmsDeliveryState state = mmMessage->deliveryState();
        // TODO does this even change?
        // TODO do something with the state
        qDebug() << "deliverystate changed" << state;
    });

    QDBusReply<void> sendResult = co_await mmMessage->send();

    if (!sendResult.isValid()) {
        updateMessageState(msgPath, MessageState::Failed);
        co_return sendResult.error().message();
    }

    co_return QString();
}

void MessageModel::markMessageRead(const int id)
{
    Q_EMIT m_handler.database().requestMarkMessageRead(id);
}

void MessageModel::disableNotifications(const QString &phoneNumber)
{
    m_handler.interface()->disableNotificationsForNumber(phoneNumber);
}
