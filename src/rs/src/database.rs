// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

use libsignal_protocol as rsp;

use crate::SignalResult;

pub struct SqliteIdentityKeyStore {}

#[async_trait(?Send)]
impl rsp::IdentityKeyStore for SqliteIdentityKeyStore {
    async fn get_identity_key_pair(&self, ctx: rsp::Context) -> SignalResult<rsp::IdentityKeyPair> {
        // TODO
        Err(rsp::SignalProtocolError::NoKeyTypeIdentifier)
    }
    async fn get_local_registration_id(&self, ctx: rsp::Context) -> SignalResult<u32> {
        // TODO
        Ok(0)
    }
    async fn save_identity(
        &mut self,
        address: &rsp::ProtocolAddress,
        identity: &rsp::IdentityKey,
        ctx: rsp::Context,
    ) -> SignalResult<bool> {
        // TODO
        Ok(true)
    }

    async fn is_trusted_identity(
        &self,
        address: &rsp::ProtocolAddress,
        identity: &rsp::IdentityKey,
        direction: rsp::Direction,
        ctx: rsp::Context,
    ) -> SignalResult<bool> {
        // TODO
        Ok(true)
    }

    async fn get_identity(
        &self,
        address: &rsp::ProtocolAddress,
        ctx: rsp::Context,
    ) -> SignalResult<Option<rsp::IdentityKey>> {
        // TODO
        Ok(None)
    }
}

pub struct SqliteSessionStore {}

#[async_trait(?Send)]
impl rsp::SessionStore for SqliteSessionStore {
    async fn load_session(
        &self,
        address: &rsp::ProtocolAddress,
        ctx: rsp::Context,
    ) -> SignalResult<Option<rsp::SessionRecord>> {
        Ok(None)
    }

    async fn store_session(
        &mut self,
        address: &rsp::ProtocolAddress,
        record: &rsp::SessionRecord,
        ctx: rsp::Context,
    ) -> SignalResult<()> {
        Ok(())
    }
}

pub struct SqlitePreKeyStore {}

#[async_trait(?Send)]
impl rsp::PreKeyStore for SqlitePreKeyStore {
    async fn get_pre_key(
        &self,
        prekey_id: u32,
        ctx: rsp::Context,
    ) -> SignalResult<rsp::PreKeyRecord> {
        Err(rsp::SignalProtocolError::NoKeyTypeIdentifier)
    }

    async fn save_pre_key(
        &mut self,
        prekey_id: u32,
        record: &rsp::PreKeyRecord,
        ctx: rsp::Context,
    ) -> SignalResult<()> {
        Ok(())
    }

    async fn remove_pre_key(&mut self, prekey_id: u32, ctx: rsp::Context) -> SignalResult<()> {
        Ok(())
    }
}

pub struct SqliteSignedPreKeyStore {}

#[async_trait(?Send)]
impl rsp::SignedPreKeyStore for SqliteSignedPreKeyStore {
    async fn get_signed_pre_key(
        &self,
        signed_prekey_id: u32,
        ctx: rsp::Context,
    ) -> SignalResult<rsp::SignedPreKeyRecord> {
        Err(rsp::SignalProtocolError::NoKeyTypeIdentifier)
    }

    async fn save_signed_pre_key(
        &mut self,
        signed_prekey_id: u32,
        record: &rsp::SignedPreKeyRecord,
        ctx: rsp::Context,
    ) -> SignalResult<()> {
        Ok(())
    }
}

pub fn new_sqlite_identity_key_store() -> Box<SqliteIdentityKeyStore> {
    Box::new(SqliteIdentityKeyStore {})
}

pub fn new_sqlite_session_store() -> Box<SqliteSessionStore> {
    Box::new(SqliteSessionStore {})
}

pub fn new_sqlite_pre_key_store() -> Box<SqlitePreKeyStore> {
    Box::new(SqlitePreKeyStore {})
}

pub fn new_sqlite_signed_pre_key_store() -> Box<SqliteSignedPreKeyStore> {
    Box::new(SqliteSignedPreKeyStore {})
}
