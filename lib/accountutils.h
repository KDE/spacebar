// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

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
