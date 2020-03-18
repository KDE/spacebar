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
        this->processRows(first, last);
    });
}

void ContactMapper::processRows(const int first, const int last)
{
    QVector<QString> affectedNumbers;
    for (int i = first; i <= last; i++) {
        const auto index = this->m_model->index(i);

        // Yes, this code has to be illogical. PersonsModel::PersonVCardRole is actually supposed
        // to return an AbstractContact::Ptr, although the name suggests differneltly. Luckily we can get
        // the actual VCard from it.
        const QByteArray vcard = this->m_model->data(index, KPeople::PersonsModel::PersonVCardRole)
                        .value<KPeople::AbstractContact::Ptr>()
                        ->customProperty(KPeople::AbstractContact::VCardProperty).toByteArray();

        const auto personUri = this->m_model->data(index, KPeople::PersonsModel::PersonUriRole).toString();

        // Note: evaluate whether catching multiple phone numbers per contact is worth
        // the performance bottleneck of parsing a vcard.
        // For now vcard is used for backends that support it,
        // otherwise the default phone number from kpeople is used
        if (vcard.isEmpty()) {
            KContacts::VCardConverter converter;
            const auto addressee = converter.parseVCard(vcard);
            const auto phoneNumbers = addressee.phoneNumbers();
            for (const auto &phoneNumber : phoneNumbers) {
                this->m_numberToUri[phoneNumber.number()] = personUri;
                affectedNumbers.append(phoneNumber.number());
            }
        } else {
            const auto phoneNumber = this->m_model->data(index, KPeople::PersonsModel::PhoneNumberRole).toString();
            this->m_numberToUri[phoneNumber] = personUri;
            affectedNumbers.append(phoneNumber);
        }
    }

    emit this->contactsChanged(affectedNumbers);
}

void ContactMapper::performInitialScan()
{
    processRows(0, this->m_model->rowCount() - 1);
}

QString ContactMapper::uriForNumber(const QString &phoneNumber) const
{
    return m_numberToUri.value(phoneNumber);
}
