// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcherBase>
#include <QObject>

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

    // This class shall never expose anything but signals

    QFuture<void> addMessage(const Message &message);
    QFuture<void> deleteMessage(const QString &id);
    [[nodiscard]] QFuture<QVector<Message>> messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id, const int &limit = 0);
    QFuture<void> updateMessageDeliveryState(const QString &id, const MessageState state);
    QFuture<void> updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    QFuture<void> markMessageRead(const int id);
    [[nodiscard]] QFuture<QVector<Chat>> chats(const PhoneNumberList &phoneNumberList);
    QFuture<void> markChatAsRead(const PhoneNumberList &phoneNumberList);
    QFuture<void> deleteChat(const PhoneNumberList &phoneNumberList);

private:
    template<typename T, typename Functor>
    QFuture<T> runAsync(Functor func)
    {
        auto interface = std::make_shared<QFutureInterface<T>>();

        QMetaObject::invokeMethod(this, [interface, func] {
            if constexpr (!std::is_same_v<T, void>) {
                auto result = func();
                interface->reportResult(result);
            } else {
                func();
            }

            interface->reportFinished();
        });

        return interface->future();
    }

    Database m_database;
};
