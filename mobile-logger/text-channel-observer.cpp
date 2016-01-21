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

#include "text-channel-observer.h"
#include "channel-watcher.h"

#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/MethodInvocationContext>
#include <TelepathyQt/Types>
#include <TelepathyQt/ReceivedMessage>

#include <KTp/types.h>
#include <KTp/contact.h>
#include <KTp/message.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

inline Tp::ChannelClassSpecList channelClasses() {
    return Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::textChat();
}

namespace KTp {
class TextChannelObserver::Private {
public:
    QHash<KTp::ContactPtr, ChannelWatcherPtr> currentChannels;
    QSqlDatabase db;
};
} //namespace

KTp::TextChannelObserver::TextChannelObserver(QObject *parent)
    : Tp::AbstractClientObserver(channelClasses(), true),
      d(new TextChannelObserver::Private)
{
    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/');
    QDir dir;
    dir.mkpath(dbLocation);

    d->db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    d->db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    if (d->db.open()) {
        const QStringList tables = d->db.tables();
        if (!tables.contains(QLatin1String("data"))) {
            QSqlQuery createTable = d->db.exec(QStringLiteral("CREATE TABLE data"
                                                              "("
                                                               "id INTEGER PRIMARY KEY, "
                                                               "messageDateTime DATETIME, "
                                                               "targetContact VARCHAR(50), "
                                                               "message TEXT, "
                                                               "isIncoming BOOLEAN, "
                                                               "isDelivered BOOLEAN, "
                                                               "type SMALLINT "
                                                            ");"));
            if (createTable.lastError().isValid()) {
                qWarning() << "Failed to create the database table:" << createTable.lastError().text();
            }
        }
    } else {
        qWarning() << "Failed to open the history database";
    }
}

KTp::TextChannelObserver::~TextChannelObserver()
{
    d->db.close();
    delete d;
}

void KTp::TextChannelObserver::observeChannels(const Tp::MethodInvocationContextPtr<> &context,
                                               const Tp::AccountPtr &account,
                                               const Tp::ConnectionPtr &connection,
                                               const QList<Tp::ChannelPtr> &channels,
                                               const Tp::ChannelDispatchOperationPtr &dispatchOperation,
                                               const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                               const Tp::AbstractClientObserver::ObserverInfo &observerInfo)
{
//     Q_UNUSED(context)
//     Q_UNUSED(account)
//     Q_UNUSED(connection)
//     Q_UNUSED(dispatchOperation)
//     Q_UNUSED(requestsSatisfied)
//     Q_UNUSED(observerInfo)

    qDebug() << "Observing channel";

    Q_FOREACH(const Tp::ChannelPtr & channel, channels) {
        Tp::TextChannelPtr textChannel = Tp::TextChannelPtr::dynamicCast(channel);
        if (textChannel) {
            KTp::ContactPtr targetContact = KTp::ContactPtr::qObjectCast(textChannel->targetContact());

            //skip group chats and situations where we don't have a single contact to mark messages for
            if (targetContact.isNull()) {
                qDebug() << "Target contact is null";
                continue;
            }

            ChannelWatcherPtr watcher = ChannelWatcherPtr(new ChannelWatcher(textChannel));
            d->currentChannels[targetContact] = watcher;

            connect(watcher.data(), SIGNAL(messagesChanged()), SLOT(onChannelMessagesChanged()));
        }
    }
}

void KTp::TextChannelObserver::onChannelMessagesChanged()
{

}

void KTp::TextChannelObserver::onChannelInvalidated()
{
//     ChannelWatcher* watcher = qobject_cast<ChannelWatcher*>(sender());
//     Q_ASSERT(watcher);
//     const QModelIndex &index = mapFromSource(watcher->modelIndex());
//     const KTp::ContactPtr &contact = index.data(KTp::ContactRole).value<KTp::ContactPtr>();

}
