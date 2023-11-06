// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QByteArray>
#include <QStringLiteral>

#define SL QStringLiteral
#define BL QByteArrayLiteral

static inline uint hash(const QString &value)
{
    uint h = 0;
    const QChar *p = value.unicode();
    size_t len = value.size();
    for (size_t i = 0; i < len; ++i) {
        h = 31 * h + p[i].unicode();
    }
    return h;
}
