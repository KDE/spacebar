// SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

mod schema;

use libsignal_protocol as rsp;

use crate::SignalResult;

use std::error::Error;
use std::fmt::{self, Display, Formatter};
use std::rc::Rc;

use diesel::prelude::*;

use diesel::ConnectionError;
use diesel_migrations::RunMigrationsError;

embed_migrations!("migrations/");

#[derive(Debug)]
pub enum DatabaseError {
    ConnectionError(ConnectionError),
    RunMigrationsError(RunMigrationsError),
}

impl Display for DatabaseError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            DatabaseError::ConnectionError(e) => write!(f, "{}", e),
            DatabaseError::RunMigrationsError(e) => write!(f, "{}", e),
        }
    }
}

impl Error for DatabaseError {}

impl From<ConnectionError> for DatabaseError {
    fn from(error: ConnectionError) -> DatabaseError {
        DatabaseError::ConnectionError(error)
    }
}

impl From<RunMigrationsError> for DatabaseError {
    fn from(error: RunMigrationsError) -> DatabaseError {
        DatabaseError::RunMigrationsError(error)
    }
}

pub type DatabaseResult<T> = Result<T, DatabaseError>;

pub struct E2eeDatabase {
    connection: SqliteConnection,
}

pub type E2eeDbPtr = Rc<E2eeDatabase>;
pub struct FFiE2eeDbPtr(E2eeDbPtr);

pub struct SqliteIdentityKeyStore {
    db: E2eeDbPtr,
}

impl E2eeDatabase {
    pub fn new(storage_location: &str) -> DatabaseResult<E2eeDatabase> {
        let connection = SqliteConnection::establish(storage_location)?;

        embedded_migrations::run_with_output(&connection, &mut std::io::stdout())?;

        Ok(E2eeDatabase { connection })
    }
}

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

impl SqliteIdentityKeyStore {
    fn new(db: E2eeDbPtr) -> Self {
        Self { db }
    }
}

pub struct SqliteSessionStore {
    db: E2eeDbPtr,
}

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

impl SqliteSessionStore {
    fn new(db: E2eeDbPtr) -> Self {
        Self { db }
    }
}

pub struct SqlitePreKeyStore {
    db: E2eeDbPtr,
}

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

impl SqlitePreKeyStore {
    fn new(db: E2eeDbPtr) -> Self {
        Self { db }
    }
}

pub struct SqliteSignedPreKeyStore {
    db: E2eeDbPtr,
}

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

impl SqliteSignedPreKeyStore {
    fn new(db: E2eeDbPtr) -> Self {
        Self { db }
    }
}

pub fn new_sqlite_identity_key_store(db: &Box<FFiE2eeDbPtr>) -> Box<SqliteIdentityKeyStore> {
    Box::new(SqliteIdentityKeyStore::new(db.0.clone()))
}

pub fn new_sqlite_session_store(db: &Box<FFiE2eeDbPtr>) -> Box<SqliteSessionStore> {
    Box::new(SqliteSessionStore::new(db.0.clone()))
}

pub fn new_sqlite_pre_key_store(db: &Box<FFiE2eeDbPtr>) -> Box<SqlitePreKeyStore> {
    Box::new(SqlitePreKeyStore::new(db.0.clone()))
}

pub fn new_sqlite_signed_pre_key_store(db: &Box<FFiE2eeDbPtr>) -> Box<SqliteSignedPreKeyStore> {
    Box::new(SqliteSignedPreKeyStore::new(db.0.clone()))
}

pub fn new_e2ee_database(storage_location: &str) -> DatabaseResult<Box<FFiE2eeDbPtr>> {
    Ok(Box::from(FFiE2eeDbPtr(Rc::new(E2eeDatabase::new(
        storage_location,
    )?))))
}
