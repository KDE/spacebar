// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatlistmodel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QQmlApplicationEngine>
#include <KPeople/PersonData>

#include <KContacts/PhoneNumber>

#include <phonenumberutils.h>

#include <global.h>
#include "messagemodel.h"
#include "utils.h"
#include "databasethread.h"
#include "channelhandler.h"

ChatListModel::ChatListModel(ChannelHandler &handler, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_mapper(ContactPhoneNumberMapper::instance())
{
    m_mapper.performInitialScan();
    connect(&m_handler.database(), &AsyncDatabase::messagesChanged, this, &ChatListModel::fetchChats);
    connect(&m_mapper, &ContactPhoneNumberMapper::contactsChanged, this, [this](const QVector<QString> &affectedNumbers) {
        qDebug() << "New data for" << affectedNumbers;
        for (const auto &number : affectedNumbers) {
            // Find the Chat object for the phone number
            const auto chatIt = std::find_if(m_chats.begin(), m_chats.end(), [&number](const Chat &chat) {
                return PhoneNumberUtils::normalize(chat.phoneNumber) == number;
            });

            int i = std::distance(m_chats.begin(), chatIt);
            const auto row = index(i);
            emit dataChanged(row, row, {Role::DisplayNameRole});
        }
    });

    connect(&m_handler.database(), &AsyncDatabase::chatsFetched, this, [this](const QVector<Chat> &chats) {
        beginResetModel();
        m_chats = chats;
        endResetModel();
        emit chatsFetched();
    });

    Q_EMIT m_handler.database().requestChats();
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
        return PhoneNumberUtils::normalize(m_chats.at(index.row()).phoneNumber);
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
    chatStarted(new MessageModel(m_handler, phoneNumber));
}

void ChatListModel::markChatAsRead(const QString &phoneNumber)
{
    Q_EMIT m_handler.database().requestMarkChatAsRead(phoneNumber);
}

void ChatListModel::fetchChats()
{
    Q_EMIT m_handler.database().requestChats();
}

void ChatListModel::deleteChat(const QString &phoneNumber)
{
    Q_EMIT m_handler.database().requestDeleteChat(phoneNumber);
}
