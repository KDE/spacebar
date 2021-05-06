// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
