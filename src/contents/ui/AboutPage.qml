// SPDX-FileTextCopyright: Michael Lang <criticaltemp@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.spacebar 1.0

FormCard.AboutPage {
    aboutData: AboutType.aboutData
    Kirigami.ColumnView.fillWidth: true
}
