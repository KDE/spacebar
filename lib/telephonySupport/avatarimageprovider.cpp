// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "avatarimageprovider.h"

#include <KPeople/PersonData>
#include <KPeopleBackend/AbstractContact>

#include <phonenumber.h>

#include "contactphonenumbermapper.h"

AvatarImageProvider::AvatarImageProvider()
    : QQuickImageProvider(QQuickImageProvider::ImageType::Image)
{
}

QImage AvatarImageProvider::requestImage(const QString &number, QSize *size, const QSize &requestedSize)
{
    const auto personData = KPeople::PersonData(ContactPhoneNumberMapper::instance().uriForNumber(PhoneNumber(number)));

    auto avatar = [&]() -> QImage {
        QVariant pic = personData.contactCustomProperty(KPeople::AbstractContact::PictureProperty);

        if (pic.canConvert<QImage>()) {
            return pic.value<QImage>();
        } else if (pic.canConvert<QUrl>()) {
            QImage image;
            image.load(pic.toUrl().toLocalFile());
            return image;
        } else {
            return {};
        }
    }();
    if (avatar.isNull()) {
        return {};
    }

    if (avatar.size().height() > requestedSize.height()) {
        avatar = avatar.scaledToHeight(requestedSize.height());
    } else if (avatar.size().width() > requestedSize.width()) {
        avatar = avatar.scaledToWidth(requestedSize.width());
    }

    if (!avatar.isNull()) {
        size->setHeight(avatar.size().height());
        size->setWidth(avatar.size().width());
    }

    return avatar;
}
