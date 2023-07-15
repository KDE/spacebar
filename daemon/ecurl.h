// SPDX-FileCopyrightText: 2022 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

#include <global.h>

#include <ares.h>
#include <curl/curl.h>

class ECurl : public QObject
{
    Q_OBJECT

public:
    explicit ECurl();
    ~ECurl();

    QByteArray networkRequest(const QString &url, const QByteArray &data = QByteArrayLiteral("")) const;

private:
    static size_t curlWriteFunction(char *chunk, size_t size, size_t len, QByteArray *response);
    static void aresResolveCallback(void *arg, int status, int timeouts, struct hostent *hostent);
    static void aresResolveWait(ares_channel channel);
};
