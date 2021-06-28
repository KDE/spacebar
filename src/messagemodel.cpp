// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagemodel.h"

#include <QDebug>
#include <QFutureWatcher>

#include <KTextToHTML>
#include <KLocalizedString>

#include <qofonomessagemanager.h>
#include <qofonomanager.h>
#include <qofonomessage.h>

#include <phonenumberutils.h>

#include <global.h>
#include <contactphonenumbermapper.h>

#include "asyncdatabase.h"
#include "channelhandler.h"
#include "utils.h"


MessageModel::MessageModel(ChannelHandler &handler, const QString &phoneNumber, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_phoneNumber(phoneNumber)
    , m_personData(new KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(phoneNumber), this))
{
    connect(&m_handler.msgManager(), &QOfonoMessageManager::incomingMessage, this, [=, this](const QString &text, const QVariantMap &info) {
        if (PhoneNumberUtils::normalize(info[SL("Sender")].toString()) != m_phoneNumber) {
            return; // Message is not for this model
        }
        Message message;
        message.id = Database::generateRandomId();
        message.read = false;
        message.text = Utils::textToHtml(text);
        message.datetime = QDateTime::fromString(info[SL("SentTime")].toString().split(QChar(u'+'))[0], Qt::ISODate);
        message.sentByMe = false;
        message.deliveryStatus = MessageState::Received; // If it arrived here, it was
        message.phoneNumber = info[SL("Sender")].toString();

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
    QString intermediateId = Database::generateRandomId();

    Message message;
    message.id = intermediateId;
    message.phoneNumber = m_phoneNumber;
    message.text = Utils::textToHtml(text);
    message.datetime = QDateTime::currentDateTime(); // NOTE: there is also tpMessage.sent(), doesn't seem to return a proper time, but maybe a backend bug?
    message.read = true; // Messages sent by us are automatically read.
    message.sentByMe = true; // only called if message sent by us.
    message.deliveryStatus = MessageState::Unknown; // if this signal is called, the message was delivered.

    // Add message to model
    addMessage(message);

    std::shared_ptr<QFutureWatcher<SendMessageResult>> watcher(
                new QFutureWatcher<SendMessageResult>(),
                [](auto *watcher) {
        watcher->deleteLater();
    });
    connect(watcher.get(), &QFutureWatcherBase::finished, this, [=, this] {
        const auto result = watcher->result();

        std::visit([=, this](auto &&result) {
            using T =  std::decay_t<decltype(result)>;

            if constexpr (std::is_same_v<T, QDBusObjectPath>) {
                const auto modelIt = std::find_if(m_messages.begin(), m_messages.end(), [&](const Message &message) {
                    return message.id == intermediateId;
                });

                if (modelIt != m_messages.cend()) {
                    const QString path = result.path();
                    modelIt->id = path;

                    const int i = (m_messages.count() - 1) - std::distance(m_messages.begin(), modelIt);

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

                    connect(ofonoMessage.get(), &QOfonoMessage::stateChanged, this, [=, this] {
                        MessageState state = parseMessageState(ofonoMessage->state());
                        modelIt->deliveryStatus = state;
                        Q_EMIT m_handler.database().requestUpdateMessageDeliveryState(path, state);
                        Q_EMIT dataChanged(index(i), index(i), {Role::DeliveryStateRole});
                    });
                } else {
                    qWarning() << "Failed to find message that was just sent. This is a bug";
                }
            } else if constexpr (std::is_same_v<T, QDBusError>) {
                Utils::instance()->showPassiveNotification(result.message());
            } else if constexpr (std::is_same_v<T, ModemNotFoundError>) {
                Utils::instance()->showPassiveNotification(i18n("The modem interface is not available"));
            }
        }, result);
    });

    watcher->setFuture(m_handler.msgManager().sendMessage(m_phoneNumber, text));
}

void MessageModel::markMessageRead(const int id)
{
    Q_EMIT m_handler.database().requestMarkMessageRead(id);
}
