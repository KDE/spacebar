// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "chatlistmodel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QQmlApplicationEngine>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>

#include <KPeople/PersonData>

#include <KContacts/PhoneNumber>

#include <global.h>
#include "channelhandler.h"
#include "messagemodel.h"
#include "utils.h"
#include "databasethread.h"

ChatListModel::ChatListModel(const ChannelHandlerPtr &handler, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_database(m_handler->database())
    , m_mapper(ContactMapper::instance())
{
    m_mapper.performInitialScan();
    connect(m_database, &AsyncDatabase::messagesChanged, this, &ChatListModel::fetchChats);
    connect(&m_mapper, &ContactMapper::contactsChanged, this, [this](const QVector<QString> &affectedNumbers) {
        qDebug() << "New data for" << affectedNumbers;
        for (const auto &number : affectedNumbers) {
            // Find the Chat object for the phone number
            const auto chatIt = std::find_if(m_chats.begin(), m_chats.end(), [&number](const Chat &chat) {
                return KContacts::PhoneNumber(chat.phoneNumber).normalizedNumber() == number;
            });

            int i = std::distance(m_chats.begin(), chatIt);
            const auto row = index(i);
            emit dataChanged(row, row, {Role::DisplayNameRole});
        }
    });

    connect(m_handler.data(), &ChannelHandler::handlerReady, this, [this] {
        m_ready = true;
        emit readyChanged();
    });

    connect(m_handler.data(), &ChannelHandler::channelOpen, this, [=](const Tp::TextChannelPtr &channel, const QString &number) {
        const auto personUri = m_mapper.uriForNumber(number);
        auto *model = new MessageModel(m_database, number, channel, personUri);
        Utils::instance()->qmlEngine()->setObjectOwnership(model, QQmlApplicationEngine::JavaScriptOwnership);
        emit chatStarted(model);
    });

    connect(m_database, &AsyncDatabase::chatsFetched, this, [this](const QVector<Chat> &chats) {
        beginResetModel();
        m_chats = chats;
        endResetModel();
        emit chatsFetched();
    });

    Q_EMIT m_database->requestChats();
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    return {
        {Role::DisplayNameRole, BL("displayName")},
        {Role::PhoneNumberRole, BL("phoneNumber")},
        {Role::LastContactedRole, BL("lastContacted")},
        {Role::UnreadMessagesRole, BL("unreadMessages")},
        {Role::PhotoRole, BL("photo")},
        {Role::LastMessageRole, BL("lastMessage")}
    };
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_chats.count()) {
        return false;
    }

    switch (role) {
    // All roles that need the personData object
    case DisplayNameRole:
        return KPeople::PersonData(m_mapper.uriForNumber(m_chats.at(index.row()).phoneNumber)).name();
    case PhotoRole:
        return KPeople::PersonData(m_mapper.uriForNumber(m_chats.at(index.row()).phoneNumber)).photo();
    case PhoneNumberRole:
        return KContacts::PhoneNumber(m_chats.at(index.row()).phoneNumber).normalizedNumber();
    case LastContactedRole:
        return m_chats.at(index.row()).lastContacted;
    case UnreadMessagesRole:
        return m_chats.at(index.row()).unreadMessages;
    case LastMessageRole:
        return m_chats.at(index.row()).lastMessage;
    }

    return {};
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_chats.count();
}

void ChatListModel::startChat(const QString &phoneNumber)
{
    m_handler->openChannel(phoneNumber);
}

void ChatListModel::markChatAsRead(const QString &phoneNumber)
{
    Q_EMIT m_database->requestMarkChatAsRead(phoneNumber);
}

void ChatListModel::fetchChats()
{
    Q_EMIT m_database->requestChats();
}

bool ChatListModel::ready() const
{
    return m_ready;
}
