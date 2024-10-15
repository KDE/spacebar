// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <optional>

#include "ecurl.h"
#include "modemcontroller.h"
#include <database.h>
#include <mms.h>

#include "qcorotask.h"

constexpr std::array<QStringView, 6> TAPBACK_KEYS = {u"♥️", u"👍", u"👎", u"😂", u"‼️", u"❓"};
constexpr std::array<QStringView, 6> TAPBACK_REMOVED = {u"Removed a heart from “",
                                                        u"Removed a like from “",
                                                        u"Removed a dislike from “",
                                                        u"Removed a laugh from “",
                                                        u"Removed an exclamation from “",
                                                        u"Removed a question mark from “"};

constexpr std::array<QStringView, 6> TAPBACK_ADDED = {u"Loved “", u"Liked “", u"Disliked “", u"Laughed at “", u"Emphasized “", u"Questioned “"};

class ChannelLogger : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.spacebar.Daemon")

public:
    explicit ChannelLogger(std::optional<QString> &modemPath, QObject *parent = nullptr);

    // QString argument since this needs to be called from DBus
    Q_SCRIPTABLE void disableNotificationsForNumber(const QString &numbers);
    Q_SCRIPTABLE void manualDownload(const QString &id, const QString &url, const QDateTime &expires);
    Q_SCRIPTABLE void sendMessage(const QString &numbers, const QString &id, const QString &text, const QStringList &files, const qint64 &totalSize);
    Q_SCRIPTABLE void sendTapback(const QString &numbers, const QString &id, const QString &tapback, const bool &isRemoved);
    Q_SCRIPTABLE void syncSettings();
    Q_SCRIPTABLE QString ownNumber();
    Q_SCRIPTABLE QString countryCode();

    /*
     * For the given phone numbers, return the list of chats associated with them.
     * Passing an empty list returns all chats.
     * @returns QList<Chat> (serialized for DBus)
     */
    Q_SCRIPTABLE StringMapList chats(const QStringList &phoneNumberList);

    Q_SCRIPTABLE void markChatAsRead(const QStringList &phoneNumberList);
    Q_SCRIPTABLE void deleteChat(const QStringList &phoneNumberList);

    /*
     * Fetch messages for the given phone numbers and id.
     * @returns QList<Message> (serialized for DBus)
     */
    Q_SCRIPTABLE StringMapList messagesForNumber(const QStringList &phoneNumberList, const QString &id, int limit);
    Q_SCRIPTABLE void updateMessageDeliveryState(const QString &id, uint state); // state is of MessageState type
    Q_SCRIPTABLE void markMessageRead(int id);
    Q_SCRIPTABLE void deleteMessage(const QString &id);

private:
    void checkMessages();
    QCoro::Task<void> handleIncomingMessage(ModemManager::Sms::Ptr msg);
    void createDownloadNotification(const MmsMessage &mmsMessage);
    QCoro::Task<void> addMessage(const Message &message);
    void updateMessage(const Message &message);
    QCoro::Task<void> saveMessage(const PhoneNumberList &phoneNumberList,
                                  const QDateTime &datetime,
                                  const QString &text = QString(),
                                  const QString &attachments = QString(),
                                  const QString &smil = QString(),
                                  const QString &fromNumber = QString(),
                                  const QString &messageId = QString(),
                                  const bool pendingDownload = false,
                                  const QString &contentLocation = QString(),
                                  const QDateTime &expires = QDateTime(),
                                  const int size = 0);
    QCoro::Task<QString> sendMessageSMS(const PhoneNumber &phoneNumber, const QString &id, const QString &text);
    QCoro::Task<QString>
    sendMessageMMS(const PhoneNumberList &phoneNumberList, const QString &id, const QString &text, const QStringList &files, const qint64 totalSize);
    QCoro::Task<void> sendCancelResponse(const QString &transactionId);
    QCoro::Task<void> sendDeliveryAcknowledgement(const QString &transactionId);
    QCoro::Task<void> sendNotifyResponse(const QString &transactionId, const QString &status);
    QCoro::Task<void> sendReadReport(const QString &messageId);
    QCoro::Task<QByteArray> uploadMessage(const QByteArray &data);
    QCoro::Task<void> downloadMessage(const MmsMessage message);
    void handleDownloadedMessage(const QByteArray &response, const QString &url, const QDateTime &expires);
    void createNotification(Message &message);
    QCoro::Task<bool> handleTapbackReaction(Message &message, const QString &reactNumber);
    QCoro::Task<bool> saveTapback(Message &message,
                                  const QString &reactNumber,
                                  const QStringView &tapback,
                                  std::span<const QStringView> list,
                                  const bool &isAdd,
                                  const bool &isImage);
    QCoro::Task<void> sendTapbackHandler(const QString &numbers, const QString &id, const QString &tapback, const bool &isRemoved);

    Database m_database;

    Mms m_mms;

    ECurl m_curl;

    PhoneNumberList m_disabledNotificationNumber;

    PhoneNumber m_ownNumber;

    bool m_dataConnected;

    QStringList m_deferredIndicators;

Q_SIGNALS:
    Q_SCRIPTABLE void messageAdded(const QString &phoneNumber, const QString &id);
    Q_SCRIPTABLE void messageUpdated(const QString &phoneNumber, const QString &id);
    Q_SCRIPTABLE void manualDownloadFinished(const QString &id, const bool isEmpty);
};
