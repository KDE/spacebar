// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>

#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/SharedPtr>

#include <contactphonenumbermapper.h>
#include "database.h"
#include "global.h"

class MessageModel;
class ChannelHandler;
class AsyncDatabase;

using ChannelHandlerPtr = Tp::SharedPtr<ChannelHandler>;

struct ChatData {
    QString displayName;
    QString phoneNumber;
    int unreadMessages;
    QDateTime lastContacted;
};

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

public:
    enum Role {
        DisplayNameRole = Qt::UserRole + 1,
        PhoneNumberRole,
        UnreadMessagesRole,
        LastContactedRole,
        PhotoRole,
        LastMessageRole
    };
    Q_ENUM(Role)

    explicit ChatListModel(const ChannelHandlerPtr &handler, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    Q_INVOKABLE void startChat(const QString &phoneNumber);
    Q_INVOKABLE void markChatAsRead(const QString &phoneNumber);

    bool ready() const;

public slots:
    void fetchChats();
    void deleteChat(const QString &phoneNumber);

signals:
    void chatStarted(MessageModel* messageModel);
    void startingChatFailed(const QString &errorMessage);

    void readyChanged();
    void chatsFetched();

private:
    ChannelHandlerPtr m_handler;
    AsyncDatabase *m_database;
    QVector<Chat> m_chats;
    ContactPhoneNumberMapper &m_mapper;
    Tp::AccountPtr m_simAccount;
    bool m_ready;
};
