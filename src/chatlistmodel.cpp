// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatlistmodel.h"

#include <QDebug>
#include <QQmlApplicationEngine>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include <QCoroFuture>

#include <KLocalizedString>
#include <KPeople/PersonData>

#include "channelhandler.h"
#include "messagemodel.h"
#include "settingsmanager.h"
#include "utils.h"
#include <database.h>
#include <global.h>

ChatListModel::ChatListModel(ChannelHandler &handler, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_mapper(ContactPhoneNumberMapper::instance())
{
    connect(&m_handler, &ChannelHandler::messagesChanged, this, &ChatListModel::fetchChats);
    connect(&m_mapper, &ContactPhoneNumberMapper::contactsChanged, this, [this](const QVector<PhoneNumber> &affectedNumbers) {
        qDebug() << "New data for" << affectedNumbers;
        for (const auto &number : affectedNumbers) {
            // Find the Chat object for the phone number
            const auto chatIt = std::find_if(m_chats.begin(), m_chats.end(), [&number](const Chat &chat) {
                return chat.phoneNumberList.contains(number);
            });

            int i = std::distance(m_chats.begin(), chatIt);
            const auto row = index(i);
            Q_EMIT dataChanged(row, row, {Role::DisplayNameRole});
        }
    });

    fetchChats(PhoneNumberList(SL("")));

    Q_EMIT m_handler.interface()->disableNotificationsForNumber(QString());
}

ChatListModel::~ChatListModel()
{
    Q_EMIT m_handler.interface()->disableNotificationsForNumber(QString());
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    return {
        {Role::DisplayNameRole, BL("displayName")},
        {Role::PhoneNumberListRole, BL("phoneNumberList")},
        {Role::UnreadMessagesRole, BL("unreadMessages")},
        {Role::LastDateTimeRole, BL("lastDateTime")},
        {Role::LastMessageRole, BL("lastMessage")},
        {Role::LastSentByMeRole, BL("lastSentByMe")},
        {Role::LastAttachmentRole, BL("lastAttachment")},
        {Role::LastContactedRole, BL("lastContacted")},
        {Role::IsContactRole, BL("isContact")},
    };
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_chats.count()) {
        return false;
    }

    PhoneNumberList phoneNumberList = m_chats.at(index.row()).phoneNumberList;

    switch (role) {
    // All roles that need the personData object
    case DisplayNameRole: {
        QString names;
        for (int i = 0; const auto &number : phoneNumberList) {
            if (!names.isEmpty()) {
                names.append(SL(", "));
            }
            QString name = KPeople::PersonData(m_mapper.uriForNumber(number)).name();
            if (name.isEmpty()) {
                name = number.toInternational();
            } else if (phoneNumberList.count() >= 2) {
                name = name.split(SL(" ")).first();
            }

            if (i > 0 && names.size() + name.size() + 5 > m_characters) {
                names.append(i18n("and %1 more", phoneNumberList.count() - i));
                break;
            }

            names.append(name);

            i++;
        }
        return names;
    }
    case PhoneNumberListRole:
        return QVariant::fromValue(phoneNumberList);
    case UnreadMessagesRole:
        return m_chats.at(index.row()).unreadMessages;
    case LastMessageRole:
        return m_chats.at(index.row()).lastMessage;
    case LastDateTimeRole:
        return m_chats.at(index.row()).lastDateTime;
    case LastSentByMeRole:
        return m_chats.at(index.row()).lastSentByMe;
    case LastAttachmentRole:
        return m_chats.at(index.row()).lastAttachment;
    case LastContactedRole: {
        QDateTime lastContacted = m_chats.at(index.row()).lastDateTime;
        if (lastContacted.daysTo(QDateTime::currentDateTime()) == 0) {
            QString format = Utils::instance()->isLocale24HourTime() ? SL("hh:mm") : SL("h:mm ap");
            return lastContacted.toString(format);
        } else {
            return lastContacted.toString(QLocale::system().dateFormat(QLocale::ShortFormat));
        }
    }
    case IsContactRole:
        return phoneNumberList.size() == 1 && !KPeople::PersonData(m_mapper.uriForNumber(phoneNumberList.first())).name().isEmpty();
    }

    return {};
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_chats.count();
}

QPair<Chat *, int> ChatListModel::getChatIndex(const PhoneNumberList &phoneNumberList)
{
    auto modelIt = std::find_if(m_chats.begin(), m_chats.end(), [&](const Chat &chat) {
        return chat.phoneNumberList.toString() == phoneNumberList.toString();
    });

    if (modelIt != m_chats.cend()) {
        const int i = std::distance(m_chats.begin(), modelIt);
        return qMakePair(&(*modelIt), i);
    } else {
        return qMakePair(&(*modelIt), -1);
    }
}

void ChatListModel::startChat(const PhoneNumberList &phoneNumberList)
{
    if (m_messageModel != nullptr)
        m_messageModel->deleteLater();

    m_messageModel = new MessageModel(m_handler, phoneNumberList, this);
    chatStarted(m_messageModel);
}

void ChatListModel::markChatAsRead(const PhoneNumberList &phoneNumberList)
{
    m_handler.database().markChatAsRead(phoneNumberList);
}

void ChatListModel::fetchChats(const PhoneNumberList &phoneNumberList)
{
    if (phoneNumberList.count() == 0) {
        fetchChatsInternal();
    } else {
        const auto &[chat, idx] = getChatIndex(phoneNumberList);

        if (idx == -1) {
            // add to chat list
            Chat chat;
            chat.phoneNumberList = phoneNumberList;

            beginInsertRows({}, 0, 0);
            m_chats.prepend(chat);
            endInsertRows();

            fetchChatDetails(phoneNumberList, false);
        } else {
            // update existing and sort
            fetchChatDetails(phoneNumberList, true);
        }
    }
}

QCoro::Task<void> ChatListModel::fetchChatsInternal()
{
    const auto chats = co_await m_handler.database().chats(PhoneNumberList());

    beginResetModel();
    m_chats = chats;
    endResetModel();

    Q_EMIT chatsFetched();
}

void ChatListModel::fetchChatDetails(const PhoneNumberList &phoneNumberList, const bool sort)
{
    [this, phoneNumberList, sort]() -> QCoro::Task<void> {
        co_await fetchChatDetailsInternal(phoneNumberList, sort);
    }();
}

QCoro::Task<void> ChatListModel::fetchChatDetailsInternal(const PhoneNumberList &phoneNumberList, const bool sort)
{
    const auto &[chat, idx] = getChatIndex(phoneNumberList);

    if (idx > -1) {
        const QVector<Chat> chats = co_await m_handler.database().chats(phoneNumberList);

        chat->phoneNumberList = chats.first().phoneNumberList;
        chat->unreadMessages = chats.first().unreadMessages;
        chat->lastMessage = chats.first().lastMessage;
        chat->lastDateTime = chats.first().lastDateTime;
        chat->lastSentByMe = chats.first().lastSentByMe;
        chat->lastAttachment = chats.first().lastAttachment;

        Q_EMIT dataChanged(index(idx), index(idx));
    }

    if (sort) {
        beginResetModel();
        // Sort chat list by most recent chat
        std::sort(m_chats.begin(), m_chats.end(), [](const Chat &a, const Chat &b) -> bool {
            return a.lastDateTime > b.lastDateTime;
        });
        endResetModel();
    }
}

void ChatListModel::deleteChat(const PhoneNumberList &phoneNumberList)
{
    m_handler.database().deleteChat(phoneNumberList);

    const QString folder = QString::number(hash(phoneNumberList.toString()));
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments/") + folder);
    dir.removeRecursively();

    const auto &[chat, idx] = getChatIndex(phoneNumberList);

    if (idx > -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_chats.remove(idx);
        endRemoveRows();
    }
}

void ChatListModel::restoreDefaults()
{
    SettingsManager::self()->setDefaults();
}

void ChatListModel::saveSettings()
{
    SettingsManager::self()->save();
    Q_EMIT m_handler.interface()->syncSettings();
}

QString ChatListModel::attachmentsFolder(const PhoneNumberList &phoneNumberList) const
{
    const QString folder = QString::number(hash(phoneNumberList.toString()));
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments/") + folder;
}

void ChatListModel::setCharacterLimit(const int &width)
{
    // 170 is the pixels taken up by other elements on the list row
    m_characters = (width - 170) / 6;
}
