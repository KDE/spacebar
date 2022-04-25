// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

fn load_keypair(public_key: Box<PublicKey>, private_key: Box<PrivateKey>) -> Box<KeyPair> {
    Box::new(KeyPair(rsp::KeyPair::new(public_key.0, private_key.0)))
}

fn new_identity_key(public_key: Box<PublicKey>) -> Box<IdentityKey> {
    Box::new(IdentityKey(rsp::IdentityKey::new(public_key.0)))
}

fn new_identity_key_pair(
    identity_key: Box<IdentityKey>,
    private_key: Box<PrivateKey>,
) -> Box<IdentityKeyPair> {
    Box::new(IdentityKeyPair(rsp::IdentityKeyPair::new(
        identity_key.0,
        private_key.0,
    )))
}

fn new_signal_message(
    message_version: u8,
    mac_key: &[u8],
    sender_ratchet_key: Box<PublicKey>,
    counter: u32,
    previous_counter: u32,
    ciphertext: &[u8],
    sender_identity_key: Box<IdentityKey>,
    receiver_identity_key: Box<IdentityKey>,
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
}

impl<'a> SignalMessageRef<'a> {
    fn body(&self) -> &[u8] {
        self.0.body()
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
    base_key: Box<PublicKey>,
    identity_key: Box<IdentityKey>,
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
    base_key: Box<PublicKey>,
    identity_key: Box<IdentityKey>,
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

        #[cxx_name = "generateKeypair"]
        fn generate_keypair() -> Box<KeyPair>;

        #[cxx_name = "loadKeypair"]
        fn load_keypair(public_key: Box<PublicKey>, private_key: Box<PrivateKey>) -> Box<KeyPair>;

        #[cxx_name = "privateKey"]
        fn private_key(self: &KeyPair) -> Box<PrivateKey>;

        #[cxx_name = "publicKey"]
        fn public_key(self: &KeyPair) -> Box<PublicKey>;

        #[cxx_name = "newIdentityKey"]
        fn new_identity_key(public_key: Box<PublicKey>) -> Box<IdentityKey>;

        #[cxx_name = "newIdentityKeyPair"]
        fn new_identity_key_pair(
            identity_key: Box<IdentityKey>,
            private_key: Box<PrivateKey>,
        ) -> Box<IdentityKeyPair>;

        #[cxx_name = "newSignalMessage"]
        fn new_signal_message(
            message_version: u8,
            mac_key: &[u8],
            sender_ratchet_key: Box<PublicKey>,
            counter: u32,
            previous_counter: u32,
            ciphertext: &[u8],
            sender_identity_key: Box<IdentityKey>,
            receiver_identity_key: Box<IdentityKey>,
        ) -> Result<Box<SignalMessage>>;

        fn body(self: &SignalMessage) -> &[u8];

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
            base_key: Box<PublicKey>,
            identity_key: Box<IdentityKey>,
            message: Box<SignalMessage>,
        ) -> Result<Box<PreKeySignalMessage>>;

        #[cxx_name = "newPreKeySignalMessage"]
        fn new_pre_key_signal_message(
            message_version: u8,
            registration_id: u32,
            signed_pre_key_id: u32,
            base_key: Box<PublicKey>,
            identity_key: Box<IdentityKey>,
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
    }
}
