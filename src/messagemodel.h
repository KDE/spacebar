// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QObject>

#include <KPeople/PersonData>

#include <phonenumberlist.h>

#include "database.h"

#include "qcorotask.h"

struct Person {
    Q_GADGET
    Q_PROPERTY(QString phoneNumber MEMBER m_phoneNumber)
    Q_PROPERTY(QString name MEMBER m_name)
public:
    QString m_phoneNumber;
    QString m_name;
};
Q_DECLARE_METATYPE(Person)

class ChannelHandler;

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PhoneNumberList phoneNumberList READ phoneNumberList NOTIFY phoneNumberListChanged)
    Q_PROPERTY(QString sendingNumber READ sendingNumber CONSTANT)
    Q_PROPERTY(QVector<Person> people READ people NOTIFY peopleChanged)
    Q_PROPERTY(QString attachmentsFolder READ attachmentsFolder CONSTANT)

public:
    enum Role {
        TextRole = Qt::UserRole + 1,
        DateRole,
        TimeRole,
        SentByMeRole,
        ReadRole,
        DeliveryStateRole,
        IdRole,
        AttachmentsRole,
        SmilRole,
        FromNumberRole,
        MessageIdRole,
        DeliveryReportRole,
        ReadReportRole,
        PendingDownloadRole,
        ContentLocationRole,
        ExpiresRole,
        ExpiresDateTimeRole,
        SizeRole,
        TapbacksRole
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

    explicit MessageModel(ChannelHandler &handler, const PhoneNumberList &phoneNumberList, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index = {}) const override;

    QVector<Person> people() const;

    PhoneNumberList phoneNumberList() const;

    QString sendingNumber() const;

    QString attachmentsFolder() const;

    Q_INVOKABLE QVariant fileInfo(const QUrl &path);

    Q_INVOKABLE void fetchAllMessages();

    /**
     * @brief adds a tapack reaction to a previously sent or recieved message,
     * and updates the model and database
     * @param id
     * @param tapback
     * @param isRemoved
     */
    Q_INVOKABLE void sendTapback(const QString &id, const QString &tapback, const bool &isRemoved);

    /**
     * @brief sends a message with the specified text,
     * and adds it to the model and database by calling addMessage(const QString&)
     * @param text
     */
    Q_INVOKABLE void sendMessage(const QString &text, const QStringList &files, const qint64 &totalSize);

    /**
     * @brief marks a message as read by calling the respective database function
     */
    Q_INVOKABLE void markMessageRead(const int id);

    /**
     * @brief downloads the contents of an MMS message
     */
    Q_INVOKABLE void downloadMessage(const QString &id, const QString &url, const QDateTime &expires);

    /**
     * @brief permanently deletes a message and any attachments
     */
    Q_INVOKABLE void deleteMessage(const QString &id, const int index, const QStringList &files);

    /**
     * @brief saves selected attachments to downloads folder
     * @param list
     */
    Q_INVOKABLE void saveAttachments(const QStringList &attachments);

    /**
     * @brief excludes set phone number from message notifications
     * @param list
     */
    Q_INVOKABLE void disableNotifications(const PhoneNumberList &phoneNumberList);

private:
    QPair<Message *, int> getMessageIndex(const QString &id);
    void updateMessageState(const QString &id, MessageState state, const bool temp = false);

    ChannelHandler &m_handler;
    std::vector<Message> m_messages;

    // properties
    PhoneNumberList m_phoneNumberList;
    QVector<Person> m_peopleData;

private Q_SLOTS:
    QCoro::Task<void> fetchMessages(const QString &id, const int limit = 0);
    QCoro::Task<void> fetchUpdatedMessage(const QString &id);
    void messageAdded(const QString &numbers, const QString &id);
    void messageUpdated(const QString &numbers, const QString &id);

Q_SIGNALS:
    void phoneNumberListChanged();
    void peopleChanged();
    void messagesFetched();
};
