// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <qofonomessagemanager.h>

#include <QFuture>

#include <optional>

class QDBusPendingCallWatcher;

/// Type that indicates that the call to the modem could not be made
struct ModemNotFoundError {};

using SendMessageResult = std::variant<QDBusObjectPath, QDBusError, ModemNotFoundError>;

///
/// Slightly improved API for QOfonoMessageManager
///
/// Allows to react to successful sending of a specific message
///
class MessageManager : public QOfonoMessageManager
{
    Q_OBJECT

public:
    MessageManager(QObject *parent = nullptr);

    /// Sends the message, and on success returns the path to the new dbus object.
    /// On error, it returns the QDBusError if it was able to call the interface, or
    /// ModemNotFoundError if not even that was possible.
    QFuture<SendMessageResult> sendMessage(const QString &to, const QString &text);
};
