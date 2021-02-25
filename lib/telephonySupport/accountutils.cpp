// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include "accountutils.h"

#include <QtGlobal>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountManager>

AccountUtils::AccountUtils()
{

}

optional<Tp::AccountPtr> AccountUtils::findTelephonyAccount(Tp::AccountManagerPtr manager)
{
    const Tp::AccountSetPtr accountSet = manager->validAccounts();
    const auto accounts = accountSet->accounts();

    auto accountIt = std::find_if(accounts.begin(), accounts.end(), [](const Tp::AccountPtr &account) {
        static const QStringList supportedProtocols = {
            QLatin1String("ofono"),
            QLatin1String("tel"),
        };

        return supportedProtocols.contains(account->protocolName());
    });

    if (accountIt != accounts.end() && !accountIt->isNull()) {
        return *accountIt;
    } else {
        qCritical() << "Unable to get SIM account;"
                    << "is the telepathy-ofono or telepathy-ring backend installed?";
    }

    return std::nullopt;
}
