// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#pragma once

#include <KPeople/PersonsModel>
#include <QObject>

#include <unordered_map>

#include "phonenumber.h"

class ContactPhoneNumberMapper : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Returns the KPeople URI belonging to phone number,
     * provided a contact exists containing the phone number.
     * If that is not the case, an empty string is returned.
     * @param phone number
     * @return the uri belonging to the phone number
     */
    QString uriForNumber(const PhoneNumber &phoneNumber) const;

    static ContactPhoneNumberMapper &instance();

Q_SIGNALS:
    /**
     * @brief contactsChanged is emitted whenever the ContactMapper has new data,
     * because a contact was added to KPeople
     * @param list of affected numbers
     */
    void contactsChanged(const QVector<PhoneNumber> phoneNumber);

private Q_SLOTS:
    void processRows(const int first, const int last);

private:
    explicit ContactPhoneNumberMapper();
    [[nodiscard]] std::string normalizeNumber(const std::string &numberString) const;

    KPeople::PersonsModel *m_model;
    QHash<PhoneNumber, QString> m_numberToUri;
    std::string m_country;
};
