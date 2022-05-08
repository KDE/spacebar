// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#[macro_use]
extern crate async_trait;

mod database;

use libsignal_protocol as rsp;

use rand_core::OsRng;

pub type SignalResult<T> = Result<T, rsp::SignalProtocolError>;

struct KeyPair(rsp::KeyPair);
struct PublicKey(rsp::PublicKey);
struct PrivateKey(rsp::PrivateKey);
struct IdentityKey(rsp::IdentityKey);
struct IdentityKeyPair(rsp::IdentityKeyPair);
struct SignalMessage(rsp::SignalMessage);
struct SignalMessageRef<'a>(&'a rsp::SignalMessage);
struct ProtocolAddress(rsp::ProtocolAddress);
struct Fingerprint(rsp::Fingerprint);
struct PreKeySignalMessage(rsp::PreKeySignalMessage);
struct Context(rsp::Context);
struct CiphertextMessage(rsp::CiphertextMessage);

// Database
use database::*;

impl KeyPair {
    fn public_key(&self) -> Box<PublicKey> {
        Box::new(PublicKey(self.0.public_key))
    }

    fn private_key(&self) -> Box<PrivateKey> {
        Box::new(PrivateKey(self.0.private_key))
    }
}

fn generate_keypair() -> Box<KeyPair> {
    Box::new(KeyPair(rsp::KeyPair::generate(&mut OsRng)))
}

fn new_keypair(public_key: &Box<PublicKey>, private_key: &Box<PrivateKey>) -> Box<KeyPair> {
    Box::new(KeyPair(rsp::KeyPair::new(public_key.0, private_key.0)))
}

fn new_identity_key(public_key: &Box<PublicKey>) -> Box<IdentityKey> {
    Box::new(IdentityKey(rsp::IdentityKey::new(public_key.0)))
}

fn new_identity_key_pair(
    identity_key: &Box<IdentityKey>,
    private_key: &Box<PrivateKey>,
) -> Box<IdentityKeyPair> {
    Box::new(IdentityKeyPair(rsp::IdentityKeyPair::new(
        identity_key.0,
        private_key.0,
    )))
}

fn deserialize_public_key(bytes: &[u8]) -> SignalResult<Box<PublicKey>> {
    Ok(Box::from(PublicKey(rsp::PublicKey::deserialize(bytes)?)))
}

impl PublicKey {
    fn serialize(&self) -> Vec<u8> {
        self.0.serialize().to_vec()
    }
}

fn deserialize_private_key(bytes: &[u8]) -> SignalResult<Box<PrivateKey>> {
    Ok(Box::from(PrivateKey(rsp::PrivateKey::deserialize(bytes)?)))
}

impl PrivateKey {
    fn serialize(&self) -> Vec<u8> {
        self.0.serialize().to_vec()
    }
}

fn new_signal_message(
    message_version: u8,
    mac_key: &[u8],
    sender_ratchet_key: &Box<PublicKey>,
    counter: u32,
    previous_counter: u32,
    ciphertext: &[u8],
    sender_identity_key: &Box<IdentityKey>,
    receiver_identity_key: &Box<IdentityKey>,
) -> SignalResult<Box<SignalMessage>> {
    Ok(Box::new(SignalMessage(rsp::SignalMessage::new(
        message_version,
        mac_key,
        sender_ratchet_key.0,
        counter,
        previous_counter,
        ciphertext,
        &sender_identity_key.0,
        &receiver_identity_key.0,
    )?)))
}

impl SignalMessage {
    fn body(&self) -> &[u8] {
        self.0.body()
    }

    fn sender_ratchet_key(&self) -> Box<PublicKey> {
        Box::from(PublicKey(self.0.sender_ratchet_key().to_owned()))
    }

    fn counter(&self) -> u32 {
        self.0.counter()
    }

    fn message_version(&self) -> u8 {
        self.0.message_version()
    }
}

impl<'a> SignalMessageRef<'a> {
    fn body(&self) -> &[u8] {
        self.0.body()
    }

    fn sender_ratchet_key(&self) -> Box<PublicKey> {
        Box::from(PublicKey(self.0.sender_ratchet_key().to_owned()))
    }

    fn counter(&self) -> u32 {
        self.0.counter()
    }

    fn message_version(&self) -> u8 {
        self.0.message_version()
    }

    fn to_owned(&self) -> Box<SignalMessage> {
        Box::from(SignalMessage(self.0.clone()))
    }
}

fn new_protocol_address(name: String, device_id: u32) -> Box<ProtocolAddress> {
    Box::new(ProtocolAddress(rsp::ProtocolAddress::new(name, device_id)))
}

impl ProtocolAddress {
    fn name(&self) -> &str {
        self.0.name()
    }

    fn device_id(&self) -> u32 {
        self.0.device_id()
    }
}

fn new_fingerprint(
    version: u32,
    iterations: u32,
    local_id: &[u8],
    local_key: &Box<IdentityKey>,
    remote_id: &[u8],
    remote_key: &Box<IdentityKey>,
) -> SignalResult<Box<Fingerprint>> {
    Ok(Box::from(Fingerprint(rsp::Fingerprint::new(
        version,
        iterations,
        local_id,
        &local_key.0,
        remote_id,
        &remote_key.0,
    )?)))
}

impl Fingerprint {
    fn display_string(&self) -> SignalResult<String> {
        self.0.display_string()
    }
}

fn new_pre_key_signal_message_with_pre_key_id(
    message_version: u8,
    registration_id: u32,
    pre_key_id: u32,
    signed_pre_key_id: u32,
    base_key: &Box<PublicKey>,
    identity_key: &Box<IdentityKey>,
    message: Box<SignalMessage>,
) -> SignalResult<Box<PreKeySignalMessage>> {
    Ok(Box::from(PreKeySignalMessage(
        rsp::PreKeySignalMessage::new(
            message_version,
            registration_id,
            Some(pre_key_id),
            signed_pre_key_id,
            base_key.0,
            identity_key.0,
            message.0,
        )?,
    )))
}

fn new_pre_key_signal_message(
    message_version: u8,
    registration_id: u32,
    signed_pre_key_id: u32,
    base_key: &Box<PublicKey>,
    identity_key: &Box<IdentityKey>,
    message: Box<SignalMessage>,
) -> SignalResult<Box<PreKeySignalMessage>> {
    Ok(Box::from(PreKeySignalMessage(
        rsp::PreKeySignalMessage::new(
            message_version,
            registration_id,
            None,
            signed_pre_key_id,
            base_key.0,
            identity_key.0,
            message.0,
        )?,
    )))
}

impl PreKeySignalMessage {
    fn message_version(&self) -> u8 {
        self.0.message_version()
    }

    fn registration_id(&self) -> u32 {
        self.0.registration_id()
    }

    fn has_pre_key_id(&self) -> bool {
        self.0.pre_key_id().is_none()
    }

    fn pre_key_id(&self) -> u32 {
        self.0
            .pre_key_id()
            .expect("Only call this if you know there is a pre key id from calling hasPreKeyId()")
    }

    fn signed_pre_key_id(&self) -> u32 {
        self.0.signed_pre_key_id()
    }

    fn base_key(&self) -> Box<PublicKey> {
        Box::from(PublicKey(*self.0.base_key()))
    }

    fn identity_key(&self) -> Box<IdentityKey> {
        Box::from(IdentityKey(*self.0.identity_key()))
    }

    fn message(&self) -> Box<SignalMessageRef> {
        Box::from(SignalMessageRef(self.0.message()))
    }

    fn serialized(&self) -> &[u8] {
        return self.0.serialized();
    }
}

fn message_encrypt(
    ptext: &[u8],
    remote_address: &ProtocolAddress,
    session_store: &mut Box<SqliteSessionStore>,
    identity_store: &mut Box<SqliteIdentityKeyStore>,
    ctx: &Context,
) -> SignalResult<Box<CiphertextMessage>> {
    let future = rsp::message_encrypt(
        ptext,
        &remote_address.0,
        session_store.as_mut(),
        identity_store.as_mut(),
        ctx.0,
    );

    futures_executor::block_on(future)
        .map(CiphertextMessage)
        .map(Box::from)
}

fn message_decrypt(
    ciphertext: &CiphertextMessage,
    remote_address: &ProtocolAddress,
    session_store: &mut Box<SqliteSessionStore>,
    identity_store: &mut Box<SqliteIdentityKeyStore>,
    pre_key_store: &mut Box<SqlitePreKeyStore>,
    signed_pre_key_store: &mut Box<SqliteSignedPreKeyStore>,
    ctx: &Context,
) -> SignalResult<Vec<u8>> {
    let rng = &mut OsRng;
    let future = rsp::message_decrypt(
        &ciphertext.0,
        &remote_address.0,
        session_store.as_mut(),
        identity_store.as_mut(),
        pre_key_store.as_mut(),
        signed_pre_key_store.as_mut(),
        rng,
        ctx.0,
    );

    futures_executor::block_on(future)
}

fn message_decrypt_prekey(
    ciphertext: &PreKeySignalMessage,
    remote_address: &ProtocolAddress,
    session_store: &mut Box<SqliteSessionStore>,
    identity_store: &mut Box<SqliteIdentityKeyStore>,
    pre_key_store: &mut Box<SqlitePreKeyStore>,
    signed_pre_key_store: &mut Box<SqliteSignedPreKeyStore>,
    ctx: &Context,
) -> SignalResult<Vec<u8>> {
    let rng = &mut OsRng;
    let future = rsp::message_decrypt_prekey(
        &ciphertext.0,
        &remote_address.0,
        session_store.as_mut(),
        identity_store.as_mut(),
        pre_key_store.as_mut(),
        signed_pre_key_store.as_mut(),
        rng,
        ctx.0,
    );

    futures_executor::block_on(future)
}

fn message_decrypt_signal(
    ciphertext: &SignalMessage,
    remote_address: &ProtocolAddress,
    session_store: &mut Box<SqliteSessionStore>,
    identity_store: &mut Box<SqliteIdentityKeyStore>,
    ctx: &Context,
) -> SignalResult<Vec<u8>> {
    let rng = &mut OsRng;
    let future = rsp::message_decrypt_signal(
        &ciphertext.0,
        &remote_address.0,
        session_store.as_mut(),
        identity_store.as_mut(),
        rng,
        ctx.0,
    );

    futures_executor::block_on(future)
}

#[cxx::bridge(namespace = "signal")]
mod ffi {
    extern "Rust" {
        type KeyPair;
        type PrivateKey;
        type PublicKey;
        type IdentityKey;
        type IdentityKeyPair;
        type SignalMessage;
        type ProtocolAddress;
        type Fingerprint;
        type PreKeySignalMessage;
        type SignalMessageRef<'a>;
        type Context;
        type CiphertextMessage;

        #[cxx_name = "generateKeypair"]
        fn generate_keypair() -> Box<KeyPair>;

        #[cxx_name = "newKeypair"]
        fn new_keypair(public_key: &Box<PublicKey>, private_key: &Box<PrivateKey>) -> Box<KeyPair>;

        #[cxx_name = "privateKey"]
        fn private_key(self: &KeyPair) -> Box<PrivateKey>;

        #[cxx_name = "publicKey"]
        fn public_key(self: &KeyPair) -> Box<PublicKey>;

        #[cxx_name = "newIdentityKey"]
        fn new_identity_key(public_key: &Box<PublicKey>) -> Box<IdentityKey>;

        #[cxx_name = "newIdentityKeyPair"]
        fn new_identity_key_pair(
            identity_key: &Box<IdentityKey>,
            private_key: &Box<PrivateKey>,
        ) -> Box<IdentityKeyPair>;

        #[cxx_name = "deserializePublicKey"]
        fn deserialize_public_key(bytes: &[u8]) -> Result<Box<PublicKey>>;
        fn serialize(self: &PublicKey) -> Vec<u8>;

        #[cxx_name = "deserializePrivateKey"]
        fn deserialize_private_key(bytes: &[u8]) -> Result<Box<PrivateKey>>;
        fn serialize(self: &PrivateKey) -> Vec<u8>;

        #[cxx_name = "newSignalMessage"]
        fn new_signal_message(
            message_version: u8,
            mac_key: &[u8],
            sender_ratchet_key: &Box<PublicKey>,
            counter: u32,
            previous_counter: u32,
            ciphertext: &[u8],
            sender_identity_key: &Box<IdentityKey>,
            receiver_identity_key: &Box<IdentityKey>,
        ) -> Result<Box<SignalMessage>>;

        fn body(self: &SignalMessage) -> &[u8];

        #[cxx_name = "senderRatchetKey"]
        fn sender_ratchet_key(self: &SignalMessage) -> Box<PublicKey>;
        #[cxx_name = "senderRatchetKey"]
        fn sender_ratchet_key(self: &SignalMessageRef) -> Box<PublicKey>;

        fn counter(self: &SignalMessage) -> u32;
        fn counter(self: &SignalMessageRef) -> u32;

        #[cxx_name = "messageVersion"]
        fn message_version(self: &SignalMessage) -> u8;
        #[cxx_name = "messageVersion"]
        fn message_version(self: &SignalMessageRef) -> u8;

        #[cxx_name = "newProtocolAddress"]
        fn new_protocol_address(name: String, device_id: u32) -> Box<ProtocolAddress>;

        fn name(self: &ProtocolAddress) -> &str;

        #[cxx_name = "deviceId"]
        fn device_id(self: &ProtocolAddress) -> u32;

        #[cxx_name = "newFingerprint"]
        fn new_fingerprint(
            version: u32,
            iterations: u32,
            local_id: &[u8],
            local_key: &Box<IdentityKey>,
            remote_id: &[u8],
            remote_key: &Box<IdentityKey>,
        ) -> Result<Box<Fingerprint>>;

        #[cxx_name = "display_string"]
        fn display_string(self: &Fingerprint) -> Result<String>;

        #[cxx_name = "newPreKeySignalMessageWithPreKeyId"]
        fn new_pre_key_signal_message_with_pre_key_id(
            message_version: u8,
            registration_id: u32,
            pre_key_id: u32,
            signed_pre_key_id: u32,
            base_key: &Box<PublicKey>,
            identity_key: &Box<IdentityKey>,
            message: Box<SignalMessage>,
        ) -> Result<Box<PreKeySignalMessage>>;

        #[cxx_name = "newPreKeySignalMessage"]
        fn new_pre_key_signal_message(
            message_version: u8,
            registration_id: u32,
            signed_pre_key_id: u32,
            base_key: &Box<PublicKey>,
            identity_key: &Box<IdentityKey>,
            message: Box<SignalMessage>,
        ) -> Result<Box<PreKeySignalMessage>>;

        #[cxx_name = "messageVersion"]
        fn message_version(self: &PreKeySignalMessage) -> u8;

        #[cxx_name = "registrationId"]
        fn registration_id(self: &PreKeySignalMessage) -> u32;

        #[cxx_name = "hasPreKeyId"]
        fn has_pre_key_id(self: &PreKeySignalMessage) -> bool;

        #[cxx_name = "preKeyId"]
        fn pre_key_id(self: &PreKeySignalMessage) -> u32;

        #[cxx_name = "signedPreKeyId"]
        fn signed_pre_key_id(self: &PreKeySignalMessage) -> u32;

        #[cxx_name = "baseKey"]
        fn base_key(self: &PreKeySignalMessage) -> Box<PublicKey>;

        #[cxx_name = "identityKey"]
        fn identity_key(self: &PreKeySignalMessage) -> Box<IdentityKey>;

        fn message(self: &PreKeySignalMessage) -> Box<SignalMessageRef>;

        fn serialized(self: &PreKeySignalMessage) -> &[u8];

        unsafe fn body<'a>(self: &'a SignalMessageRef<'a>) -> &'a [u8];

        #[cxx_name = "toOwned"]
        unsafe fn to_owned<'a>(self: &'a SignalMessageRef) -> Box<SignalMessage>;

        #[cxx_name = "messageEncrypt"]
        fn message_encrypt(
            ptext: &[u8],
            remote_address: &ProtocolAddress,
            session_store: &mut Box<SqliteSessionStore>,
            identity_store: &mut Box<SqliteIdentityKeyStore>,
            ctx: &Context,
        ) -> Result<Box<CiphertextMessage>>;

        #[cxx_name = "messageDecrypt"]
        fn message_decrypt(
            ciphertext: &CiphertextMessage,
            remote_address: &ProtocolAddress,
            session_store: &mut Box<SqliteSessionStore>,
            identity_store: &mut Box<SqliteIdentityKeyStore>,
            pre_key_store: &mut Box<SqlitePreKeyStore>,
            signed_pre_key_store: &mut Box<SqliteSignedPreKeyStore>,
            ctx: &Context,
        ) -> Result<Vec<u8>>;

        #[cxx_name = "messageDecryptPrekey"]
        fn message_decrypt_prekey(
            ciphertext: &PreKeySignalMessage,
            remote_address: &ProtocolAddress,
            session_store: &mut Box<SqliteSessionStore>,
            identity_store: &mut Box<SqliteIdentityKeyStore>,
            pre_key_store: &mut Box<SqlitePreKeyStore>,
            signed_pre_key_store: &mut Box<SqliteSignedPreKeyStore>,
            ctx: &Context,
        ) -> Result<Vec<u8>>;

        #[cxx_name = "messageDecryptSignal"]
        fn message_decrypt_signal(
            ciphertext: &SignalMessage,
            remote_address: &ProtocolAddress,
            session_store: &mut Box<SqliteSessionStore>,
            identity_store: &mut Box<SqliteIdentityKeyStore>,
            ctx: &Context,
        ) -> Result<Vec<u8>>;

        // Database
        type SqliteIdentityKeyStore;
        type SqliteSessionStore;
        type SqliteSignedPreKeyStore;
        type SqlitePreKeyStore;

        #[cxx_name = "newSqliteIdentityKeyStore"]
        fn new_sqlite_identity_key_store() -> Box<SqliteIdentityKeyStore>;

        #[cxx_name = "newSqliteSessionStore"]
        fn new_sqlite_session_store() -> Box<SqliteSessionStore>;

        #[cxx_name = "newSqlitePreKeyStore"]
        fn new_sqlite_pre_key_store() -> Box<SqlitePreKeyStore>;

        #[cxx_name = "newSqliteSignedPreKeyStore"]
        fn new_sqlite_signed_pre_key_store() -> Box<SqliteSignedPreKeyStore>;
    }
}
