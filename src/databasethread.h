// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QThread>

#include <asyncdatabase.h>

///
/// \brief The DatabaseThread class is a preconfigured QThread
///
class DatabaseThread : public QThread
{
    Q_OBJECT

public:
    explicit DatabaseThread(QObject *parent = nullptr);
    ~DatabaseThread();

    AsyncDatabase &database();

private:
    AsyncDatabase m_database;
};
