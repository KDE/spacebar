// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcherBase>

#include "database.h"

template <typename T, typename QObjectDerivedType, typename Function>
void connectFuture(const QFuture<T> &future, QObjectDerivedType *self, const Function &fun) {
    auto watcher = std::make_shared<QFutureWatcher<T>>();
    watcher->setFuture(future);
    QObject::connect(watcher.get(), &QFutureWatcherBase::finished, self, [self, watcher, fun, future] {
        if constexpr (std::is_same_v<void, T>) {
            if constexpr (std::is_member_function_pointer_v<Function>) {
                fun->*(self);
            } else {
                fun();
            }
        } else if (future.resultCount() > 0) {
            if constexpr (std::is_member_function_pointer_v<Function>) {
                (self->*fun)(watcher->result());
            } else {
                fun(watcher->result());
            }
        }
    });
}

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

    Q_SIGNAL void messagesChanged(const PhoneNumberList &phoneNumberList);

    QFuture<void> addMessage(const Message &message);
    QFuture<void> deleteMessage(const QString &id);
    QFuture<QVector<Message>> messagesForNumber(const PhoneNumberList &phoneNumberList, const QString &id);
    QFuture<void> updateMessageDeliveryState(const QString &id, const MessageState state);
    QFuture<void> updateMessageSent(const QString &id, const QString &messageId, const QString &contentLocation);
    QFuture<void> markMessageRead(const int id);
    QFuture<QVector<Chat>> chats();
    QFuture<int> unreadMessagesForNumber(const PhoneNumberList &phoneNumberList);
    QFuture<QString> lastMessageForNumber(const PhoneNumberList &phoneNumberList);
    QFuture<QDateTime> lastContactedForNumber(const PhoneNumberList &phoneNumberList);
    QFuture<void> markChatAsRead(const PhoneNumberList &phoneNumberList);
    QFuture<void> deleteChat(const PhoneNumberList &phoneNumberList);

private:
    template <typename T, typename Functor>
    QFuture<T> runAsync(Functor func) {
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
