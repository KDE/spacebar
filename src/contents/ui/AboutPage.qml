// SPDX-FileTextCopyright: Michael Lang <criticaltemp@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.spacebar

FormCard.AboutPage {
    aboutData: AboutType.aboutData
    Kirigami.ColumnView.fillWidth: true
}
