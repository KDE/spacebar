// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>

#include "qcorotask.h"

#include "global.h"
#include <contactphonenumbermapper.h>
#include <database.h>

class MessageModel;
class ChannelHandler;

class ChatListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ~ChatListModel();

    enum Role {
        DisplayNameRole = Qt::UserRole + 1,
        PhoneNumberListRole,
        UnreadMessagesRole,
        LastMessageRole,
        LastDateTimeRole,
        LastSentByMeRole,
        LastAttachmentRole,
        LastContactedRole,
        IsContactRole
    };
    Q_ENUM(Role)

    explicit ChatListModel(ChannelHandler &handler, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    Q_INVOKABLE void fetchChatDetails(const PhoneNumberList &phoneNumberList, const bool sort = false);
    Q_INVOKABLE void startChat(const PhoneNumberList &phoneNumberList);
    Q_INVOKABLE void markChatAsRead(const PhoneNumberList &phoneNumberList);
    Q_INVOKABLE void restoreDefaults();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE QString attachmentsFolder(const PhoneNumberList &phoneNumberList) const;
    Q_INVOKABLE void setCharacterLimit(const int &width);

    bool ready() const;

public Q_SLOTS:
    void fetchChats(const PhoneNumberList &phoneNumberList);
    void deleteChat(const PhoneNumberList &phoneNumberList);

Q_SIGNALS:
    void chatStarted(MessageModel *messageModel);
    void startingChatFaild(const QString &errorMessage);

    void readyChanged();
    void chatsFetched();

private:
    QPair<Chat *, int> getChatIndex(const PhoneNumberList &phoneNumberList);
    QCoro::Task<void> fetchChatsInternal();
    QCoro::Task<void> fetchChatDetailsInternal(const PhoneNumberList &phoneNumberList, const bool sort = false);

    ChannelHandler &m_handler;
    QVector<Chat> m_chats;
    ContactPhoneNumberMapper &m_mapper;
    MessageModel *m_messageModel = nullptr;
    int m_characters = 15;
};
