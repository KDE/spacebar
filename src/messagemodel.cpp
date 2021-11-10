// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QFutureWatcher>

#include <KTextToHTML>
#include <KLocalizedString>

#include <ModemManagerQt/Sms>

#include <global.h>
#include <contactphonenumbermapper.h>
#include <phonenumberlist.h>

#include "asyncdatabase.h"
#include "channelhandler.h"
#include "utils.h"
#include "modemcontroller.h"

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

    connect(&ModemController::instance(), &ModemController::messageAdded, this, [this](ModemManager::Sms::Ptr msg) {
        if (PhoneNumberList(msg->number()) != m_phoneNumberList) {
            return; // Message is not for this model
        }
        Message message;
        message.id = Database::generateRandomId();
        message.read = false;
        message.text = Utils::textToHtml(msg->text());
        message.datetime = msg->timestamp();
        message.sentByMe = false;
        message.deliveryStatus = MessageState::Received; // If it arrived here, it was
        message.phoneNumberList = PhoneNumberList(msg->number());

        addMessage(message);
    });

    connect(&m_handler.database(), &AsyncDatabase::messagesFetchedForNumber,
            this, [this](const PhoneNumberList &phoneNumberList, const QVector<Message> &messages) {
        if (phoneNumberList == m_phoneNumberList) {
            beginResetModel();
            m_messages = messages;
            endResetModel();
        }
    });

    Q_EMIT m_handler.database().requestMessagesForNumber(m_phoneNumberList);
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

QVector<Person> MessageModel::people() const
{
    return m_peopleData;
}

PhoneNumberList MessageModel::phoneNumberList() const
{
    return m_phoneNumberList;
}

void MessageModel::addMessage(const Message &message)
{
    beginInsertRows({}, m_messages.count(), m_messages.count());
    m_messages.prepend(message);
    endInsertRows();

    // save to database
    Q_EMIT m_handler.database().requestAddMessage(message);
}

void MessageModel::sendMessage(const QString &text)
{
    [this, text] () -> QCoro::Task<void> {
        QString result;
        if (m_phoneNumberList.size() > 1) {
            // send as individual messages
            for (const auto &phoneNumber : m_phoneNumberList) {
                result += co_await sendMessageInternal(phoneNumber, text);
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

QCoro::Task<QString> MessageModel::sendMessageInternal(const PhoneNumber &phoneNumber, const QString &text)
{
    ModemManager::ModemMessaging::Message m;
    m.number = phoneNumber.toE164();
    m.text = text;//Utils::textToHtml(text);

    Message message;
    message.phoneNumberList = PhoneNumberList(phoneNumber.toInternational());
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

void MessageModel::deleteMessage(const QString &id, const int index)
{
    Q_EMIT m_handler.database().requestDeleteMessage(id);

    beginRemoveRows(QModelIndex(), index, index);
    m_messages.remove(m_messages.count() - index - 1);
    endRemoveRows();
}

void MessageModel::disableNotifications(const PhoneNumberList &phoneNumberList)
{
    m_handler.interface()->disableNotificationsForNumber(phoneNumberList.toString());
}
