#pragma once

#include <QObject>
#include <QAbstractListModel>

#include "database.h"

class MessageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        TextRole = Qt::UserRole + 1,
        TimeRole,
        SentByMeRole,
        ReadRole
    };
    Q_ENUM(Role)

    explicit MessageModel(Database *database, const QString &phoneNumber, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

private:
    Database *m_database;
    QVector<Message> m_messages;
    QString m_phoneNumber;
};
