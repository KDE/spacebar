table! {
    identities (address) {
        address -> Text,
        identity_key -> Binary,
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
    pre_keys (prekey_id) {
        prekey_id -> BigInt,
        pre_key -> Binary,
    }
}

table! {
    sessions (previous_counter) {
        previous_counter -> Integer,
        session_state -> Binary,
    }
}

table! {
    signed_pre_keys (signed_prekey_id) {
        signed_prekey_id -> BigInt,
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
