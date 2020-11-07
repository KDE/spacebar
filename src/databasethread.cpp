// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "databasethread.h"

#include <QDebug>

#include <global.h>

DatabaseThread::DatabaseThread(QObject *parent)
    : QThread(parent)
{
    m_database.moveToThread(this);
    setObjectName(SL("DatabaseThread"));
    start();
}

DatabaseThread::~DatabaseThread()
{
    quit();
    wait();
}

AsyncDatabase *DatabaseThread::database()
{
    return &m_database;
}
