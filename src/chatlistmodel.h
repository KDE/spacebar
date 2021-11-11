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
        PhoneNumberListRole,
        UnreadMessagesRole,
        LastContactedRole,
        LastMessageRole,
        IsContactRole
    };
    Q_ENUM(Role)

    explicit ChatListModel(ChannelHandler &handler, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    Q_INVOKABLE void startChat(const PhoneNumberList &phoneNumberList);
    Q_INVOKABLE void markChatAsRead(const PhoneNumberList &phoneNumberList);
    Q_INVOKABLE void restoreDefaults();
    Q_INVOKABLE void saveSettings();

    bool ready() const;

public Q_SLOTS:
    void fetchChats();
    void deleteChat(const PhoneNumberList &phoneNumberList);

Q_SIGNALS:
    void chatStarted(MessageModel* messageModel);
    void startingChatFaild(const QString &errorMessage);

    void readyChanged();
    void chatsFetched();

private:
    void setChats(const QVector<Chat> &chats);

    ChannelHandler &m_handler;
    QVector<Chat> m_chats;
    ContactPhoneNumberMapper &m_mapper;
    MessageModel *m_messageModel = nullptr;
};
