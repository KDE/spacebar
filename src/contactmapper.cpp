#include "contactmapper.h"

#include <QDebug>
#include <KContacts/VCardConverter>
#include <QThread>

ContactMapper::ContactMapper(QObject *parent)
    : QObject(parent)
    , m_model(new KPeople::PersonsModel(this))
{
    // data updates
    // we only care about additional data, not remove one
    connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        this->processRows(first, last);
    });
}

void ContactMapper::processRows(int first, int last)
{
    QVector<QString> affectedNumbers;
    for (int i = first; i <= last; i++) {
        const auto index = m_model->index(i);

        const auto vcard = m_model->data(index, KPeople::PersonsModel::PersonVCardRole).toByteArray();
        const auto personUri = m_model->data(index, KPeople::PersonsModel::PersonUriRole).toString();

        // Note: evaluate whether catching multiple phone numbers per contact is worth
        // the performance bottleneck of parsing a vcard.
        // For now vcard is used for backends that support it,
        // otherwise the default phone number from kpeople is used
        if (vcard.isEmpty()) {
            KContacts::VCardConverter converter;
            auto addressee = converter.parseVCard(vcard);
            for (const auto &phoneNumber : addressee.phoneNumbers()) {
                m_numberToUri[phoneNumber.number()] = personUri;
                affectedNumbers.append(phoneNumber.number());
            }
        } else {
            auto phoneNumber = m_model->data(index, KPeople::PersonsModel::PhoneNumberRole).toString();
            m_numberToUri[phoneNumber] = personUri;
            affectedNumbers.append(phoneNumber);
        }
    }

    emit this->contactsChanged(affectedNumbers);
}

void ContactMapper::performInitialScan()
{
    processRows(0, m_model->rowCount());
}

QString ContactMapper::uriForNumber(const QString &phoneNumber) const
{
    return m_numberToUri.value(phoneNumber);
}
