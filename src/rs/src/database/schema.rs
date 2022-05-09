// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

table! {
    identities (address) {
        address -> Text,
        identity_key -> Binary,
        trusted_incoming -> Bool,
        trusted_outgoing -> Bool,
    }
}

table! {
    own_identities (registration_id) {
        registration_id -> BigInt,
        identity_key -> Binary,
        private_key -> Binary,
    }
}

table! {
    pre_keys (pre_key_id) {
        pre_key_id -> BigInt,
        pre_key -> Binary,
    }
}

table! {
    sessions (address) {
        address -> Text,
        session_state -> Binary,
    }
}

table! {
    signed_pre_keys (signed_pre_key_id) {
        signed_pre_key_id -> BigInt,
        signed_pre_key -> Binary,
    }
}

allow_tables_to_appear_in_same_query!(
    identities,
    own_identities,
    pre_keys,
    sessions,
    signed_pre_keys,
);
