// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

use libsignal_protocol::{
    Fingerprint, IdentityKey, IdentityKeyPair, KeyPair, PreKeySignalMessage, PrivateKey,
    ProtocolAddress, PublicKey, SignalMessage, SignalProtocolError,
};

use rand::rngs::OsRng;

pub type SignalResult<T> = Result<T, SignalProtocolError>;

struct SignalKeyPair(KeyPair);
struct SignalPublicKey(PublicKey);
struct SignalPrivateKey(PrivateKey);
struct SignalIdentityKey(IdentityKey);
struct SignalIdentityKeyPair(IdentityKeyPair);
struct SignalSignalMessage(SignalMessage);
struct SignalSignalMessageRef<'a>(&'a SignalMessage);
struct SignalProtocolAddress(ProtocolAddress);
struct SignalFingerprint(Fingerprint);
struct SignalPreKeySignalMessage(PreKeySignalMessage);

impl SignalKeyPair {
    fn public_key(&self) -> Box<SignalPublicKey> {
        Box::new(SignalPublicKey(self.0.public_key))
    }

    fn private_key(&self) -> Box<SignalPrivateKey> {
        Box::new(SignalPrivateKey(self.0.private_key))
    }
}

fn generate_keypair() -> Box<SignalKeyPair> {
    Box::new(SignalKeyPair(KeyPair::generate(&mut OsRng)))
}

fn load_keypair(
    public_key: Box<SignalPublicKey>,
    private_key: Box<SignalPrivateKey>,
) -> Box<SignalKeyPair> {
    Box::new(SignalKeyPair(KeyPair::new(public_key.0, private_key.0)))
}

fn new_identity_key(public_key: Box<SignalPublicKey>) -> Box<SignalIdentityKey> {
    Box::new(SignalIdentityKey(IdentityKey::new(public_key.0)))
}

fn new_identity_key_pair(
    identity_key: Box<SignalIdentityKey>,
    private_key: Box<SignalPrivateKey>,
) -> Box<SignalIdentityKeyPair> {
    Box::new(SignalIdentityKeyPair(IdentityKeyPair::new(
        identity_key.0,
        private_key.0,
    )))
}

fn new_signal_message(
    message_version: u8,
    mac_key: &[u8],
    sender_ratchet_key: Box<SignalPublicKey>,
    counter: u32,
    previous_counter: u32,
    ciphertext: &[u8],
    sender_identity_key: Box<SignalIdentityKey>,
    receiver_identity_key: Box<SignalIdentityKey>,
) -> SignalResult<Box<SignalSignalMessage>> {
    Ok(Box::new(SignalSignalMessage(SignalMessage::new(
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

impl SignalSignalMessage {
    fn body(&self) -> &[u8] {
        self.0.body()
    }
}

impl<'a> SignalSignalMessageRef<'a> {
    fn body(&self) -> &[u8] {
        self.0.body()
    }

    fn to_owned(&self) -> Box<SignalSignalMessage> {
        Box::from(SignalSignalMessage(self.0.clone()))
    }
}

fn new_protocol_address(name: String, device_id: u32) -> Box<SignalProtocolAddress> {
    Box::new(SignalProtocolAddress(ProtocolAddress::new(name, device_id)))
}

impl SignalProtocolAddress {
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
    local_key: &Box<SignalIdentityKey>,
    remote_id: &[u8],
    remote_key: &Box<SignalIdentityKey>,
) -> SignalResult<Box<SignalFingerprint>> {
    Ok(Box::from(SignalFingerprint(Fingerprint::new(
        version,
        iterations,
        local_id,
        &local_key.0,
        remote_id,
        &remote_key.0,
    )?)))
}

impl SignalFingerprint {
    fn display_string(&self) -> SignalResult<String> {
        self.0.display_string()
    }
}

fn new_pre_key_signal_message_with_pre_key_id(
    message_version: u8,
    registration_id: u32,
    pre_key_id: u32,
    signed_pre_key_id: u32,
    base_key: Box<SignalPublicKey>,
    identity_key: Box<SignalIdentityKey>,
    message: Box<SignalSignalMessage>,
) -> SignalResult<Box<SignalPreKeySignalMessage>> {
    Ok(Box::from(SignalPreKeySignalMessage(
        PreKeySignalMessage::new(
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
    base_key: Box<SignalPublicKey>,
    identity_key: Box<SignalIdentityKey>,
    message: Box<SignalSignalMessage>,
) -> SignalResult<Box<SignalPreKeySignalMessage>> {
    Ok(Box::from(SignalPreKeySignalMessage(
        PreKeySignalMessage::new(
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

impl SignalPreKeySignalMessage {
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

    fn base_key(&self) -> Box<SignalPublicKey> {
        Box::from(SignalPublicKey(*self.0.base_key()))
    }

    fn identity_key(&self) -> Box<SignalIdentityKey> {
        Box::from(SignalIdentityKey(*self.0.identity_key()))
    }

    fn message(&self) -> Box<SignalSignalMessageRef> {
        Box::from(SignalSignalMessageRef(self.0.message()))
    }

    fn serialized(&self) -> &[u8] {
        return self.0.serialized();
    }
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type SignalKeyPair;
        type SignalPrivateKey;
        type SignalPublicKey;
        type SignalIdentityKey;
        type SignalIdentityKeyPair;
        type SignalSignalMessage;
        type SignalProtocolAddress;
        type SignalFingerprint;
        type SignalPreKeySignalMessage;
        type SignalSignalMessageRef<'a>;

        #[cxx_name = "generateKeypair"]
        fn generate_keypair() -> Box<SignalKeyPair>;

        #[cxx_name = "loadKeypair"]
        fn load_keypair(
            public_key: Box<SignalPublicKey>,
            private_key: Box<SignalPrivateKey>,
        ) -> Box<SignalKeyPair>;

        #[cxx_name = "privateKey"]
        fn private_key(self: &SignalKeyPair) -> Box<SignalPrivateKey>;

        #[cxx_name = "publicKey"]
        fn public_key(self: &SignalKeyPair) -> Box<SignalPublicKey>;

        #[cxx_name = "newIdentityKey"]
        fn new_identity_key(public_key: Box<SignalPublicKey>) -> Box<SignalIdentityKey>;

        #[cxx_name = "newIdentityKeyPair"]
        fn new_identity_key_pair(
            identity_key: Box<SignalIdentityKey>,
            private_key: Box<SignalPrivateKey>,
        ) -> Box<SignalIdentityKeyPair>;

        #[cxx_name = "newSignalMessage"]
        fn new_signal_message(
            message_version: u8,
            mac_key: &[u8],
            sender_ratchet_key: Box<SignalPublicKey>,
            counter: u32,
            previous_counter: u32,
            ciphertext: &[u8],
            sender_identity_key: Box<SignalIdentityKey>,
            receiver_identity_key: Box<SignalIdentityKey>,
        ) -> Result<Box<SignalSignalMessage>>;

        fn body(self: &SignalSignalMessage) -> &[u8];

        #[cxx_name = "newProtocolAddress"]
        fn new_protocol_address(name: String, device_id: u32) -> Box<SignalProtocolAddress>;

        fn name(self: &SignalProtocolAddress) -> &str;

        #[cxx_name = "deviceId"]
        fn device_id(self: &SignalProtocolAddress) -> u32;

        #[cxx_name = "newFingerprint"]
        fn new_fingerprint(
            version: u32,
            iterations: u32,
            local_id: &[u8],
            local_key: &Box<SignalIdentityKey>,
            remote_id: &[u8],
            remote_key: &Box<SignalIdentityKey>,
        ) -> Result<Box<SignalFingerprint>>;

        #[cxx_name = "display_string"]
        fn display_string(self: &SignalFingerprint) -> Result<String>;

        #[cxx_name = "newPreKeySignalMessageWithPreKeyId"]
        fn new_pre_key_signal_message_with_pre_key_id(
            message_version: u8,
            registration_id: u32,
            pre_key_id: u32,
            signed_pre_key_id: u32,
            base_key: Box<SignalPublicKey>,
            identity_key: Box<SignalIdentityKey>,
            message: Box<SignalSignalMessage>,
        ) -> Result<Box<SignalPreKeySignalMessage>>;

        #[cxx_name = "newPreKeySignalMessage"]
        fn new_pre_key_signal_message(
            message_version: u8,
            registration_id: u32,
            signed_pre_key_id: u32,
            base_key: Box<SignalPublicKey>,
            identity_key: Box<SignalIdentityKey>,
            message: Box<SignalSignalMessage>,
        ) -> Result<Box<SignalPreKeySignalMessage>>;

        #[cxx_name = "messageVersion"]
        fn message_version(self: &SignalPreKeySignalMessage) -> u8;

        #[cxx_name = "registrationId"]
        fn registration_id(self: &SignalPreKeySignalMessage) -> u32;

        #[cxx_name = "hasPreKeyId"]
        fn has_pre_key_id(self: &SignalPreKeySignalMessage) -> bool;

        #[cxx_name = "preKeyId"]
        fn pre_key_id(self: &SignalPreKeySignalMessage) -> u32;

        #[cxx_name = "signedPreKeyId"]
        fn signed_pre_key_id(self: &SignalPreKeySignalMessage) -> u32;

        #[cxx_name = "baseKey"]
        fn base_key(self: &SignalPreKeySignalMessage) -> Box<SignalPublicKey>;

        #[cxx_name = "identityKey"]
        fn identity_key(self: &SignalPreKeySignalMessage) -> Box<SignalIdentityKey>;

        fn message(self: &SignalPreKeySignalMessage) -> Box<SignalSignalMessageRef>;

        fn serialized(self: &SignalPreKeySignalMessage) -> &[u8];

        unsafe fn body<'a>(self: &'a SignalSignalMessageRef<'a>) -> &'a [u8];

        #[cxx_name = "toOwned"]
        unsafe fn to_owned<'a>(self: &'a SignalSignalMessageRef) -> Box<SignalSignalMessage>;
    }
}
