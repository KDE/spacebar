/*
 * Copyright (C) 2013 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2016 Martin Klapetek <mklapetek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "channel-watcher.h"

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/types.h>
#include <KTp/contact.h>
#include <KTp/message.h>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

ChannelWatcher::ChannelWatcher(const Tp::TextChannelPtr &channel, const QString &accountObjectPath, QObject *parent)
    : QObject(parent),
      m_channel(channel),
      m_accountObjectPath(accountObjectPath),
      m_db(QSqlDatabase::database()),
      m_contactDbId(0), //sqlite auto-increment starts at 1
      m_accountDbId(0)
{
    qDebug() << "Delivery reports support" << channel->deliveryReportingSupport();

    connect(channel.data(), &Tp::TextChannel::invalidated, this, &ChannelWatcher::invalidated);
    connect(channel.data(), &Tp::TextChannel::invalidated, this, [=]() {
        qDebug() << "Channel invalidated";
    });

    connect(channel.data(), &Tp::TextChannel::messageReceived, this, &ChannelWatcher::onMessageReceived);
    connect(channel.data(), &Tp::TextChannel::messageSent, this, &ChannelWatcher::onMessageSent);

    qDebug() << this << "New channel being watched" << channel.data();

    storeContactInfo();
    storeAccountInfo();

    Q_FOREACH (const Tp::ReceivedMessage &message, channel->messageQueue()) {
        onMessageReceived(message);
    }
}

ChannelWatcher::~ChannelWatcher()
{
}

void ChannelWatcher::storeContactInfo()
{
    QSqlQuery insertContactIdQuery;
    insertContactIdQuery.prepare(QStringLiteral("INSERT INTO contactData VALUES (NULL, :contactId);"));
    insertContactIdQuery.bindValue(QStringLiteral(":contactId"), m_channel->targetContact()->id());

    if (!m_db.transaction()) {
        qWarning() << "Cannot get a transaction lock for inserting contact data!";
    }

    if (insertContactIdQuery.exec()) {
        m_db.commit();
        m_contactDbId = insertContactIdQuery.lastInsertId().toUInt();
    } else {
        qWarning() << "Inserting contact data into database has failed:" << insertContactIdQuery.lastError().text();
        m_db.rollback();

        // Now we assume here that it failed because of the UNIQUE constraint
        // so try to get the id from the database, assuming it already exists
        QSqlQuery selectContactId;
        selectContactId.prepare(QStringLiteral("SELECT id FROM contactData WHERE targetContact = :targetContact"));
        selectContactId.bindValue(QStringLiteral(":targetContact"), m_channel->targetContact()->id());
        selectContactId.exec();

        if (!selectContactId.lastError().isValid() && selectContactId.first()) {
            m_contactDbId = selectContactId.value(0).toUInt();
        } else {
            qWarning() << "Error while getting contact id from database:" << selectContactId.lastError().text();
        }
    }
}

void ChannelWatcher::storeAccountInfo()
{
    QSqlQuery insertAccountObjectPathQuery;
    insertAccountObjectPathQuery.prepare(QStringLiteral("INSERT INTO accountData VALUES (NULL, :accountObjectPath);"));
    insertAccountObjectPathQuery.bindValue(QStringLiteral(":accountObjectPath"), m_accountObjectPath);

    if (!m_db.transaction()) {
        // no special handling is required as commit()/rollback() will do nothing
        // if no transaction exists
        qWarning() << "Cannot get a transaction lock for inserting account data!";
    }

    if (insertAccountObjectPathQuery.exec()) {
        m_db.commit();
        m_accountDbId = insertAccountObjectPathQuery.lastInsertId().toUInt();
    } else {
        qWarning() << "Inserting account data into database has failed:" << insertAccountObjectPathQuery.lastError().text();
        m_db.rollback();

        // Now we assume here that it failed because of the UNIQUE constraint
        // so try to get the id from the database, assuming it already exists
        QSqlQuery selectAccountId;
        selectAccountId.prepare(QStringLiteral("SELECT id FROM accountData WHERE accountObjectPath = :accountObjectPath"));
        selectAccountId.bindValue(QStringLiteral(":accountObjectPath"), m_accountObjectPath);
        selectAccountId.exec();

        if (!selectAccountId.lastError().isValid() && selectAccountId.first()) {
            m_accountDbId = selectAccountId.value(0).toUInt();
        } else {
            qWarning() << "Error while getting account id from database:" << selectAccountId.lastError().text();
        }
    }
}

void ChannelWatcher::onMessageReceived(const Tp::ReceivedMessage &message)
{
    if (!message.isDeliveryReport()) {
        StorageMessage msg;
        msg.messageDateTime = message.received();
        msg.accountObjectPathId = m_accountDbId;
        msg.targetContactId = m_contactDbId;
        msg.message = message.text();
        msg.messageToken = message.messageToken();
        msg.isIncoming = true;
        msg.isDelivered = true;
        msg.type = 1;

        storeMessage(msg);
    } else {
        qDebug() << "Received a delivery report for message" << message.deliveryDetails().originalToken();

        //TODO
        //     QSqlQuery updateQuery;
        //     updateQuery.prepare("UPDATE data SET deliveredDateTime = :deliveredDateTime, isDelivered = :isDelivered WHERE id = :id");
        //     updateQuery.bindValue(":deliveredDateTime", message.deliveredDateTime);
        //     updateQuery.bindValue(":isDelivered", message.isDelivered);
        //     updateQuery.bindValue(":id", message.id);
        //
        //     bool transactionBegin = d->db.transaction();
        //     qDebug() << "Update transaction begins" << transactionBegin;
        //     bool queryResult = updateQuery.exec();
        //     qDebug() << "Update query gut" << queryResult;
        //     if (queryResult) {
        //         d->db.commit();
        //     } else {
        //         qWarning() << updateQuery.lastError().text();
        //         d->db.rollback();
        //     }

    }
}

void ChannelWatcher::onMessageSent(const Tp::Message &message)
{
    StorageMessage msg;
    msg.messageDateTime = message.sent();
    msg.accountObjectPathId = m_accountDbId;
    msg.targetContactId = m_contactDbId;
    msg.message = message.text();
    msg.messageToken = message.messageToken();
    msg.isIncoming = false;
    msg.isDelivered = false;
    msg.type = 1;

    storeMessage(msg);
}

void ChannelWatcher::storeMessage(const StorageMessage &message)
{
    QSqlQuery storeQuery;
    storeQuery.prepare("INSERT INTO data VALUES (NULL, :messageDateTime, NULL, :accountObjectPath, :targetContact, :message, :messageToken, :isIncoming, :isDelivered, :type)");
    storeQuery.bindValue(":messageDateTime", message.messageDateTime);
    storeQuery.bindValue(":accountObjectPath", message.accountObjectPathId);
    storeQuery.bindValue(":targetContact", message.targetContactId);
    storeQuery.bindValue(":message", message.message);
    storeQuery.bindValue(":messageToken", message.messageToken);
    storeQuery.bindValue(":isIncoming", message.isIncoming);
    storeQuery.bindValue(":isDelivered", message.isDelivered);
    storeQuery.bindValue(":type", message.type);

    if (m_db.transaction()) {
        qWarning() << "Cannot get a transaction lock for inserting the message!";
    }

    if (storeQuery.exec()) {
        m_db.commit();
    } else {
        qWarning() << "Error when inserting the message to the database:" << storeQuery.lastError().text();
        m_db.rollback();
    }
}
