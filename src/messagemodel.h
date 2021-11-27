// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QAbstractListModel>

#include <KPeople/PersonData>

#include <phonenumberlist.h>

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
Q_DECLARE_METATYPE(Person)

class ChannelHandler;

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(PhoneNumberList phoneNumberList READ phoneNumberList NOTIFY phoneNumberListChanged)
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
        SizeRole
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
                          const PhoneNumberList &phoneNumberList,
                          QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index = {}) const override;

    QVector<Person> people() const;

    PhoneNumberList phoneNumberList() const;

    QString attachmentsFolder() const;

    Q_INVOKABLE QVariant fileInfo(const QString &path);

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
    Q_INVOKABLE void sendMessage(const QString &text, const QStringList &files, const long totalSize);

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
    QCoro::Task<QString> sendMessageInternal(const PhoneNumber &phoneNumber, const QString &text);
    QCoro::Task<QString> sendMessageInternalMms(const PhoneNumberList &phoneNumberList, const QString &text, const QStringList &files, const long totalSize);
    QPair<Message *, int> getMessageIndex(const QString &path);
    void updateMessageState(const QString &path, MessageState state, const bool temp = false);

    ChannelHandler &m_handler;
    QVector<Message> m_messages;

    // properties
    PhoneNumberList m_phoneNumberList;
    QVector<Person> m_peopleData;

private Q_SLOTS:
    void messagedAdded(const QString &numbers, const QString &id);

Q_SIGNALS:
    void phoneNumberListChanged();
    void peopleChanged();
};
