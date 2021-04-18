// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <qofonomessagemanager.h>

#include <QFuture>

class QDBusPendingCallWatcher;

class SOfonoMessageManager : public QOfonoMessageManager
{
    Q_OBJECT

public:
    SOfonoMessageManager(QObject *parent = nullptr);

    QFuture<std::tuple<bool, QString>> sendMessage(const QString &to, const QString &text);
};
