// SPDX-FileCopyrightText: 2020 Anthony Fieroni <bvbfan@abv.bg>
// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chatlistmodel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QQmlApplicationEngine>

#include <QCoroFuture>

#include <KLocalizedString>
#include <KPeople/PersonData>

#include <global.h>
#include "messagemodel.h"
#include "utils.h"
#include "databasethread.h"
#include "channelhandler.h"
#include "settingsmanager.h"

ChatListModel::ChatListModel(ChannelHandler &handler, QObject *parent)
    : QAbstractListModel(parent)
    , m_handler(handler)
    , m_mapper(ContactPhoneNumberMapper::instance())
{
    connect(&m_handler.database(), &AsyncDatabase::messagesChanged, this, &ChatListModel::fetchChats);
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

    fetchChats();

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
        {Role::LastContactedRole, BL("lastContacted")},
        {Role::UnreadMessagesRole, BL("unreadMessages")},
        {Role::LastMessageRole, BL("lastMessage")},
        {Role::LastSentByMeRole, BL("lastSentByMe")},
        {Role::LastAttachmentRole, BL("lastAttachment")},
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
        case DisplayNameRole:
        {
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
    case LastContactedRole: {
        QDateTime lastContacted = m_chats.at(index.row()).lastContacted;
        if (lastContacted.daysTo(QDateTime::currentDateTime()) == 0) {
            QString format = Utils::instance()->isLocale24HourTime() ? SL("hh:mm") : SL("h:mm ap");
            return lastContacted.toString(format);
        } else {
            return lastContacted.toString(QLocale::system().dateFormat(QLocale::ShortFormat));
        }
    }
    case UnreadMessagesRole:
        return m_chats.at(index.row()).unreadMessages;
    case LastMessageRole:
        return m_chats.at(index.row()).lastMessage;
    case LastSentByMeRole:
        return m_chats.at(index.row()).lastSentByMe;
    case LastAttachmentRole:
        return m_chats.at(index.row()).lastAttachment;
    case IsContactRole:
        return phoneNumberList.size() == 1 && !KPeople::PersonData(m_mapper.uriForNumber(phoneNumberList.first())).name().isEmpty();
    }

    return {};
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_chats.count();
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

void ChatListModel::fetchChats()
{
    fetchChatsInternal();
}

QCoro::Task<void> ChatListModel::fetchChatsInternal()
{
    const auto chats = co_await m_handler.database().chats();

    Q_EMIT chatsFetched();

    beginResetModel();
    m_chats = chats;
    // Sort chat list by most recent chat
    std::sort(m_chats.begin(), m_chats.end(), [](const Chat &a, const Chat &b) -> bool {
        return a.lastContacted > b.lastContacted;
    });
    endResetModel();
}

void ChatListModel::deleteChat(const PhoneNumberList &phoneNumberList)
{
    m_handler.database().deleteChat(phoneNumberList);

    const QString folder = QString::number(qHash(phoneNumberList.toString()));
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments/") + folder);
    dir.removeRecursively();
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
    const QString folder = QString::number(qHash(phoneNumberList.toString()));
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + SL("/spacebar/attachments/") + folder;
}

void ChatListModel::setCharacterLimit(const int &width)
{
    // 170 is the pixels taken up by other elements on the list row
    m_characters = (width - 170) / 6;
}
