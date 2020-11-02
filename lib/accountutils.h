#pragma once

#include <TelepathyQt/Account>

#include <optional>

using std::optional;

class AccountUtils
{
public:
    AccountUtils();

    static optional<Tp::AccountPtr> findTelephonyAccount(Tp::AccountManagerPtr manager);
};
