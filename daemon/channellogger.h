// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <qofonomanager.h>
#include <qofonomessagemanager.h>

#include <database.h>

class ChannelLogger : public QObject
{
    Q_OBJECT

public:
    explicit ChannelLogger(QObject *parent = nullptr);

private:
    void handleIncomingMessage(const QString &text, const QVariantMap &info);

    Database m_database;

    QOfonoManager m_manager;
    QOfonoMessageManager m_msgManager;

signals:
    void handlerReady();
};
