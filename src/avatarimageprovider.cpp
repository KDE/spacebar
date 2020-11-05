// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "avatarimageprovider.h"

#include <KPeople/PersonData>
#include <KPeopleBackend/AbstractContact>

#include "contactmapper.h"

AvatarImageProvider::AvatarImageProvider()
    : QQuickImageProvider(QQuickImageProvider::ImageType::Pixmap)
{

}

QPixmap AvatarImageProvider::requestPixmap(const QString &number, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    const auto personData = KPeople::PersonData(ContactMapper::instance().uriForNumber(number));

    QPixmap avatar;
    QVariant pic = personData.contactCustomProperty(KPeople::AbstractContact::PictureProperty);
    if (pic.canConvert<QImage>()) {
        avatar = QPixmap::fromImage(pic.value<QImage>());
    } else if (pic.canConvert<QUrl>()) {
        avatar = QPixmap(pic.toUrl().toLocalFile());
    }

    if (!avatar.isNull()) {
        size->setHeight(avatar.size().height());
        size->setWidth(avatar.size().width());

        return avatar;
    }

    return {};
}
