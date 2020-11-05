// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQuickImageProvider>

class AvatarImageProvider : public QQuickImageProvider
{
public:
    explicit AvatarImageProvider();

    QPixmap requestPixmap(const QString &number, QSize *size, const QSize &requestedSize) override;
};
