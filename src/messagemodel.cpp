#include "messagemodel.h"

#include <QDebug>

#include "global.h"

MessageModel::MessageModel(Database *database, const QString &phoneNumber, QObject *parent)
    : QAbstractListModel(parent)
{
    this->m_database = database;
    this->m_phoneNumber = phoneNumber;

    this->m_messages = this->m_database->messagesForNumber(m_phoneNumber);
    qDebug() << "ROWS" << this->rowCount({}) << "for" << phoneNumber;

    for (const auto &message : m_messages) {
        qDebug() << message.text;
    }

    connect(m_database, &Database::messagesChanged, this, [this](const QString &phoneNumber) {
        // Only refresh model if messages are concerning our phone number
        // TODO: Incremental loading from the database
        if (phoneNumber == m_phoneNumber) {
            beginResetModel();
            this->m_messages = this->m_database->messagesForNumber(this->m_phoneNumber);
            endResetModel();
        }
    });
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
    if (!index.isValid() || index.row() < 0 || index.row() >= this->m_messages.count()) {
        return false;
    }

    switch (role) {
    case Role::TextRole:
        return this->m_messages.at(index.row()).text;
    case Role::TimeRole:
        return this->m_messages.at(index.row()).time;
    case Role::SentByMeRole:
        return this->m_messages.at(index.row()).sentByMe;
    case Role::ReadRole:
        return this->m_messages.at(index.row()).read;
    }

    return {};
}

int MessageModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : m_messages.count();
}
