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

#include <KPeople/PersonData>

#include "global.h"

ChatListModel::ChatListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_database(new Database(this))
    , m_chats(m_database->chats())
    , m_mapper(new ContactMapper(this))
{
    m_mapper->performInitialScan();
    connect(m_database, &Database::messagesChanged, this, &ChatListModel::fetchChats);
    connect(m_mapper, &ContactMapper::contactsChanged, this, [this](const QVector<QString> &affectedNumbers) {
        qDebug() << "New data for" << affectedNumbers;
        for (const auto &uri : affectedNumbers) {
            for (int i = 0; i < m_chats.count(); i++) {
                if (m_chats.at(i).phoneNumber == uri) {
                    const auto row = this->index(i);
                    emit this->dataChanged(row, row, {Role::DisplayNameRole});
                }
            }
        }
    });

    // Set up sms account
    QEventLoop loop;
    Tp::AccountManagerPtr manager = Tp::AccountManager::create();
    Tp::PendingReady *ready = manager->becomeReady();
    QObject::connect(ready, &Tp::PendingReady::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    const Tp::AccountSetPtr accountSet = manager->validAccounts();
    const auto accounts = accountSet->accounts();
    for (const Tp::AccountPtr &account : accounts) {
        static const QStringList supportedProtocols = {
            QLatin1String("ofono"),
            QLatin1String("tel"),
        };
        if (supportedProtocols.contains(account->protocolName())) {
            this->m_simAccount = account;
            break;
        }
    }

    if (m_simAccount.isNull()) {
        qCritical() << "Unable to get SIM account;"
                    << "is the telepathy-ofono or telepathy-ring backend installed?";
    }
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    return {
        {Role::DisplayNameRole, BL("displayName")},
        {Role::PhoneNumberRole, BL("phoneNumber")},
        {Role::LastContactedRole, BL("lastContacted")},
        {Role::UnreadMessagesRole, BL("unreadMessages")},
        {Role::PhotoRole, BL("photo")}
    };
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= this->m_chats.count()) {
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
        const auto photo = person->photo();
        delete person;
        return photo;
    }
    // everything else
    case PhoneNumberRole:
        return this->m_chats.at(index.row()).phoneNumber;
    case LastContactedRole:
        return this->m_chats.at(index.row()).lastContacted;
    case UnreadMessagesRole:
        return this->m_chats.at(index.row()).unreadMessages;
    };

    return {};
}

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : this->m_chats.count();
}

void ChatListModel::startChat(const QString &phoneNumber)
{
    auto *pendingRequest = this->m_simAccount->ensureTextChat(phoneNumber);
    connect(pendingRequest, &Tp::PendingChannelRequest::finished, pendingRequest, [=]() {
        if (pendingRequest->isError()) {
            qWarning() << "Error while requesting channel" << pendingRequest->errorMessage();
        }
        if (pendingRequest->channelRequest()) {
            if (pendingRequest->channelRequest()->channel()) {
                auto channel = pendingRequest->channelRequest()->channel();
                channel->becomeReady();
                qDebug() << "channel is ready" << channel->isReady();
            }
        }
    });

    // TODO: add logic
    auto *model = new MessageModel(m_database, phoneNumber, this);
    emit chatStarted(model);
}

void ChatListModel::fetchChats()
{
    beginResetModel();
    this->m_chats = m_database->chats();
    endResetModel();
}
