-- SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
--
-- SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

create table identities (
    address Text primary key not null,
    identity_key Blob not null,
    trusted_incoming Boolean not null,
    trusted_outgoing Boolean not null
);

create table own_identities (
    registration_id BigInt primary key not null,
    identity_key Blob not null,
    private_key Blob not null
);

create table sessions (
    address Text primary key not null,
    session_state Blob not null
);

create table pre_keys (
    pre_key_id BigInt primary key not null,
    pre_key Blob not null
);

create table signed_pre_keys (
    signed_pre_key_id BigInt primary key not null,
    signed_pre_key Blob not null
);
