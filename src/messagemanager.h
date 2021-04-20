// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <qofonomessagemanager.h>

#include <QFuture>

class QDBusPendingCallWatcher;

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

    QFuture<std::pair<bool, QString>> sendMessage(const QString &to, const QString &text);
};
