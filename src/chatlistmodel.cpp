#include "chatlistmodel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QThread>

#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>

#include <KPeople/PersonData>

#include <KContacts/PhoneNumber>

#include "global.h"
#include "channelhandler.h"
#include "messagemodel.h"

ChatListModel::ChatListModel(const ChannelHandlerPtr &handler)
    : m_database(new Database(this))
    , m_chats(m_database->chats())
    , m_mapper(new ContactMapper(this))
    , m_handler(handler)
{
    m_mapper->performInitialScan();
    connect(m_database, &Database::messagesChanged, this, &ChatListModel::fetchChats);
    connect(m_mapper, &ContactMapper::contactsChanged, this, [this](const QVector<QString> &affectedNumbers) {
        qDebug() << "New data for" << affectedNumbers;
        for (const auto &number : affectedNumbers) {
            // Find the Chat object for the phone number
            for (int i = 0; i < m_chats.count(); i++) {
                if (KContacts::PhoneNumber(m_chats.at(i).phoneNumber).normalizedNumber() == number) {
                    const auto row = index(i);
                    emit dataChanged(row, row, {Role::DisplayNameRole});
                }
            }
        }
    });

    connect(m_handler.data(), &ChannelHandler::channelOpen, this, [=](const Tp::TextChannelPtr& channel, const QString &number) {
        const auto personUri = m_mapper->uriForNumber(number);
        auto *model = new MessageModel(m_database, number, channel, personUri);
        emit chatStarted(model);
    });
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
    case DisplayNameRole: {
        const auto *person = new KPeople::PersonData(m_mapper->uriForNumber(m_chats.at(index.row()).phoneNumber));
        const QString name = person->name();
        delete person;
        return name;
    }
    case PhotoRole: {
        const auto *person = new KPeople::PersonData(m_mapper->uriForNumber(m_chats.at(index.row()).phoneNumber));
        const QPixmap photo = person->photo();
        delete person;
        return photo;
    }
    // everything else
    case PhoneNumberRole:
        return m_chats.at(index.row()).phoneNumber;
    case LastContactedRole:
        return m_chats.at(index.row()).lastContacted;
    case UnreadMessagesRole:
        return m_chats.at(index.row()).unreadMessages;
    case LastMessageRole:
        return m_chats.at(index.row()).lastMessage;
    };

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
    m_database->markChatAsRead(phoneNumber);
}

void ChatListModel::fetchChats()
{
    beginResetModel();
    m_chats = m_database->chats();
    endResetModel();
}
