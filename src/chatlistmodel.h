// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

﻿#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>
#include <TelepathyQt/Account>
#include <TelepathyQt/TextChannel>

#include "contactmapper.h"
#include "database.h"
#include "global.h"

class MessageModel;
class ChannelHandler;

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

    explicit ChatListModel(const ChannelHandlerPtr &handler);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    Q_INVOKABLE void startChat(const QString &phoneNumber);
    Q_INVOKABLE void markChatAsRead(const QString &phoneNumber);

    bool ready() const;

private slots:
    void fetchChats();

signals:
    void chatStarted(MessageModel* messageModel);
    void startingChatFailed(const QString &errorMessage);

    void readyChanged();

private:
    ChannelHandlerPtr m_handler;
    Database *m_database;
    QVector<Chat> m_chats;
    ContactMapper *m_mapper;
    Tp::AccountPtr m_simAccount;
    bool m_ready;
};
