// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQuickImageProvider>

class AvatarImageProvider : public QQuickImageProvider
{
public:
    explicit AvatarImageProvider();

    QImage requestImage(const QString &number, QSize *size, const QSize &requestedSize) override;
};
