-- SPDX-FileCopyrightText: 2023 Jonah Brüchert <jbb@kaidan.im>
--
-- SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

CREATE TABLE Messages (
    id TEXT,
    phoneNumber TEXT,
    text TEXT,
    time DATETIME,
    read BOOLEAN,
    delivered INTEGER,
    sentByMe BOOLEAN,
    attachments TEXT,
    smil TEXT,
    fromNumber TEXT,
    messageId TEXT,
    deliveryReport INTEGER,
    readReport TEXT,
    pendingDownload BOOLEAN,
    contentLocation TEXT,
    expires DATETIME,
    size INTEGER,
    tapbacks TEXT
);
