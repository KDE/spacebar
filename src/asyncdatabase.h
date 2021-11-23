// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QFuture>

#include "database.h"


///
/// \brief The AsyncDatabase class provides an asynchronous API around the Database class
///
/// It can be used for running the Database on a separate thread.
///
class AsyncDatabase : public QObject
{
    Q_OBJECT

public:
    explicit AsyncDatabase();

    Q_SIGNAL void messagesChanged(const PhoneNumberList &phoneNumber);

    // All of these functions are thread-safe, and are processed in an internal queue.
    QFuture<void> addMessage(const Message &message);
    QFuture<void> deleteMessage(const QString &id);
    QFuture<QVector<Message>> messagesForNumber(const PhoneNumberList &phoneNumber, const QString &id);
    QFuture<void> updateMessageDeliveryState(const QString &id, const MessageState state);
    QFuture<void> updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    QFuture<void> markMessageRead(const int id);
    QFuture<QVector<Chat>> chats();
    QFuture<int> unreadMessagesForNumber(const PhoneNumberList &phoneNumber);
    QFuture<QString> lastMessageForNumber(const PhoneNumberList &phoneNumber);
    QFuture<QDateTime> lastContactedForNumber(const PhoneNumberList &phoneNumber);
    QFuture<void> markChatAsRead(const PhoneNumberList &phoneNumber);
    QFuture<void> deleteChat(const PhoneNumberList &phoneNumber);

private:
    template <typename T, typename Func>
    QFuture<T> invokeOnThread(Func fun) {
        auto interface = std::make_shared<QFutureInterface<T>>();
        QMetaObject::invokeMethod(this, [fun, interface] {
            if constexpr (std::is_same_v<T, void>) {
                fun();
                interface->reportFinished();
            } else {
                interface->reportResult(fun());
                interface->reportFinished();
            }
        }, Qt::QueuedConnection);

        return interface->future();
    }

    Database m_database;
};
