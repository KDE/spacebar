// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "contactphonenumbermapper.h"

#include <KContacts/VCardConverter>
#include <KPeopleBackend/AbstractContact>
#include <QDebug>

#include <phonenumberutils.h>

ContactPhoneNumberMapper::ContactPhoneNumberMapper()
    : QObject()
    , m_model(new KPeople::PersonsModel(this))
{
    // data updates
    // we only care about additional rows, not removed ones
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        processRows(first, last);
    });
}

void ContactPhoneNumberMapper::processRows(const int first, const int last)
{
    QVector<QString> affectedNumbers;
    for (int i = first; i <= last; i++) {
        const auto index = m_model->index(i);

        const auto phoneNumbers = m_model->data(index, KPeople::PersonsModel::PersonVCardRole)
                        .value<KPeople::AbstractContact::Ptr>()
                        ->customProperty(KPeople::AbstractContact::AllPhoneNumbersProperty).toStringList();

        const auto personUri = m_model->data(index, KPeople::PersonsModel::PersonUriRole).toString();

        for (const auto &number : phoneNumbers) {
            const auto normalizedNumber = PhoneNumberUtils::normalize(number);
            m_numberToUri[normalizedNumber] = personUri;
            affectedNumbers.append(std::move(normalizedNumber));
        }
    }

    emit contactsChanged(affectedNumbers);
}

void ContactPhoneNumberMapper::performInitialScan()
{
    processRows(0, m_model->rowCount() - 1);
}

ContactPhoneNumberMapper &ContactPhoneNumberMapper::instance()
{
    static ContactPhoneNumberMapper instance;

    return instance;
}
