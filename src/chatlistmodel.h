#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QDateTime>
#include <TelepathyQt/Account>

#include "database.h"
#include "contactmapper.h"

struct ChatData {
    QString displayName;
    QString phoneNumber;
    int unreadMessages;
    QDateTime lastContacted;
};

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        DisplayNameRole = Qt::UserRole + 1,
        PhoneNumberRole,
        UnreadMessagesRole,
        LastContactedRole
    };
    Q_ENUM(Role)

    explicit ChatListModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    Q_INVOKABLE void startChat(const QString &phoneNumber);

private slots:
    void fetchChats();

private:
    Database *m_database;
    QVector<Chat> m_chats;
    ContactMapper *m_mapper;
    Tp::AccountPtr m_simAccount;
};
