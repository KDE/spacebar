// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QAbstractListModel>

#include <KPeople/PersonData>

#include <phonenumber.h>

#include "database.h"

#include "qcoro/task.h"

class ChannelHandler;

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PhoneNumber phoneNumber READ phoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(QString displayPhoneNumber READ displayPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(KPeople::PersonData *person READ person NOTIFY personChanged)

public:
    enum Role {
        TextRole = Qt::UserRole + 1,
        DateRole,
        TimeRole,
        SentByMeRole,
        ReadRole,
        DeliveryStateRole,
        IdRole
    };
    Q_ENUM(Role)

    enum DeliveryState {
        Unknown = MessageState::Unknown,
        Pending = MessageState::Pending,
        Sent = MessageState::Sent,
        Failed = MessageState::Failed,
        Received = MessageState::Received
    };
    Q_ENUM(DeliveryState)

    explicit MessageModel(ChannelHandler &handler,
                          const PhoneNumber &phoneNumber,
                          QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index = {}) const override;

    KPeople::PersonData *person() const;

    PhoneNumber phoneNumber() const;

    QString displayPhoneNumber() const;

    /**
     * @brief Adds a message to the model and the database.
     * Can be used for example when a new message is received.
     * @param message
     */
    void addMessage(const Message &message);

    /**
     * @brief sends a message with the specified text,
     * and adds it to the model and database by calling addMessage(const QString&)
     * @param text
     */
    Q_INVOKABLE void sendMessage(const QString &text);

    /**
     * @brief marks a message as read by calling the respective database function
     */
    Q_INVOKABLE void markMessageRead(const int id);

    /**
     * @brief permanently deletes a message
     */
    Q_INVOKABLE void deleteMessage(const QString &id, const int index);

    /**
     * @brief excludes set phone number from message notifications
     * @param list
     */
    Q_INVOKABLE void disableNotifications(const PhoneNumber &phoneNumber);

private:
    QCoro::Task<QString> sendMessageInternal(const QString &number);
    QPair<Message *, int> getMessageIndex(const QString &path);
    void updateMessageState(const QString &path, MessageState state);

    ChannelHandler &m_handler;
    QVector<Message> m_messages;

    // properties
    PhoneNumber m_phoneNumber;
    KPeople::PersonData *m_personData;

signals:
    void phoneNumberChanged();
    void personChanged();
};
