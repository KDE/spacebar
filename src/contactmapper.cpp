// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "contactmapper.h"

#include <KContacts/VCardConverter>
#include <KPeopleBackend/AbstractContact>
#include <QDebug>
#include <QThread>

ContactMapper::ContactMapper(QObject *parent)
    : QObject(parent)
    , m_model(new KPeople::PersonsModel(this))
{
    // data updates
    // we only care about additional data, not remove one
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        processRows(first, last);
    });
}

void ContactMapper::processRows(const int first, const int last)
{
    QVector<QString> affectedNumbers;
    for (int i = first; i <= last; i++) {
        const auto index = m_model->index(i);

        // Yes, this code has to be illogical. PersonsModel::PersonVCardRole is actually supposed
        // to return an AbstractContact::Ptr, although the name suggests differneltly. Luckily we can get
        // the actual VCard from it.
        const auto phoneNumbers = m_model->data(index, KPeople::PersonsModel::PersonVCardRole)
                        .value<KPeople::AbstractContact::Ptr>()
                        ->customProperty(KPeople::AbstractContact::AllPhoneNumbersProperty).toStringList();

        const auto personUri = m_model->data(index, KPeople::PersonsModel::PersonUriRole).toString();

        for (const auto &number : phoneNumbers) {
            const auto normalizedNumber = KContacts::PhoneNumber(number).normalizedNumber();
            m_numberToUri[normalizedNumber] = personUri;
            affectedNumbers.append(normalizedNumber);
        }
    }

    emit contactsChanged(affectedNumbers);
}

void ContactMapper::performInitialScan()
{
    processRows(0, m_model->rowCount() - 1);
}

QString ContactMapper::uriForNumber(const QString &phoneNumber) const
{
    return m_numberToUri.value(KContacts::PhoneNumber(phoneNumber).normalizedNumber());
}
