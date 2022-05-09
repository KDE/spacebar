create table identities (
    address Text primary key not null,
    identity_key Blob not null
);

create table own_identities (
    registration_id BigInt primary key not null,
    identity_key Blob not null,
    private_key Blob not null
);

create table sessions (
    previous_counter Int primary key not null,
    session_state Blob not null
);

create table pre_keys (
    prekey_id BigInt primary key not null,
    pre_key Blob not null
);

create table signed_pre_keys (
    signed_prekey_id BigInt primary key not null,
    signed_pre_key Blob not null
);
