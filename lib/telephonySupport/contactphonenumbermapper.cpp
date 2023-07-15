// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "contactphonenumbermapper.h"

#include <KPeopleBackend/AbstractContact>
#include <QDebug>
#include <QThread>

#include <phonenumber.h>

ContactPhoneNumberMapper &ContactPhoneNumberMapper::instance()
{
    static ContactPhoneNumberMapper instance;
    return instance;
}

ContactPhoneNumberMapper::ContactPhoneNumberMapper()
    : QObject()
    , m_model(new KPeople::PersonsModel(this))
{
    // data updates
    // we only care about additional data, not remove one
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        processRows(first, last);
    });

    processRows(0, m_model->rowCount() - 1);
}

void ContactPhoneNumberMapper::processRows(const int first, const int last)
{
    QVector<PhoneNumber> affectedNumbers;
    for (int i = first; i <= last; i++) {
        const auto index = m_model->index(i);

        // Yes, this code has to be illogical. PersonsModel::PersonVCardRole is actually supposed
        // to return an AbstractContact::Ptr, although the name suggests differneltly. Luckily we can get
        // the actual VCard from it.
        const auto phoneNumbers = m_model->data(index, KPeople::PersonsModel::PersonVCardRole)
                                      .value<KPeople::AbstractContact::Ptr>()
                                      ->customProperty(KPeople::AbstractContact::AllPhoneNumbersProperty)
                                      .toStringList();

        const auto personUri = m_model->data(index, KPeople::PersonsModel::PersonUriRole).toString();

        for (const QString &numberString : phoneNumbers) {
            const auto phoneNum = PhoneNumber(numberString);
            if (phoneNum.isValid()) {
                m_numberToUri[phoneNum] = personUri;
                affectedNumbers.append(phoneNum);
            }
        }
    }

    Q_EMIT contactsChanged(affectedNumbers);
}

QString ContactPhoneNumberMapper::uriForNumber(const PhoneNumber &phoneNumber) const
{
    if (phoneNumber.isValid()) {
        if (m_numberToUri.contains(phoneNumber)) {
            return m_numberToUri.value(phoneNumber);
        }
    }

    return QString();
}
