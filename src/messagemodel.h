// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QAbstractListModel>

#include <KPeople/PersonData>

#include <phonenumberset.h>

#include "database.h"

#include "qcoro/task.h"

struct Person {
    Q_GADGET
    Q_PROPERTY(QString phoneNumber MEMBER m_phoneNumber)
    Q_PROPERTY(QString name MEMBER m_name)
public:
    QString m_phoneNumber;
    QString m_name;
};
Q_DECLARE_METATYPE(Person);

class ChannelHandler;

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PhoneNumberSet phoneNumberList READ phoneNumberList NOTIFY phoneNumberListChanged)
    Q_PROPERTY(QVector<Person> people READ people NOTIFY peopleChanged)

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
                          const PhoneNumberSet &phoneNumberList,
                          QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index = {}) const override;

    QVector<Person> people() const;

    PhoneNumberSet phoneNumberList() const;

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
    Q_INVOKABLE void disableNotifications(const PhoneNumberSet &phoneNumberList);

private:
    QCoro::Task<QString> sendMessageInternal(const PhoneNumber &phoneNumber, const QString &text);
    QPair<Message *, int> getMessageIndex(const QString &path);
    void updateMessageState(const QString &path, MessageState state);

    ChannelHandler &m_handler;
    QVector<Message> m_messages;

    // properties
    PhoneNumberSet m_phoneNumberList;
    QVector<Person> m_peopleData;

signals:
    void phoneNumberListChanged();
    void peopleChanged();
};
