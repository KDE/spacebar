// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>

#include <contactphonenumbermapper.h>
#include "database.h"
#include "global.h"
#include "databasethread.h"

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

public:
    ~ChatListModel();

    enum Role {
        DisplayNameRole = Qt::UserRole + 1,
        DisplayPhoneNumberRole,
        PhoneNumberRole,
        UnreadMessagesRole,
        LastContactedRole,
        PhotoRole,
        LastMessageRole
    };
    Q_ENUM(Role)

    explicit ChatListModel(ChannelHandler &handler, QObject *parent = nullptr);

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
    void startingChatFaild(const QString &errorMessage);

    void readyChanged();
    void chatsFetched();

private:
    ChannelHandler &m_handler;
    QVector<Chat> m_chats;
    ContactPhoneNumberMapper &m_mapper;
    MessageModel *m_messageModel = nullptr;
};
