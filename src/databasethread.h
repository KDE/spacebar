// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

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

    AsyncDatabase *database();

private:
    AsyncDatabase m_database;
};
