/*
 * Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef KTP_MOBILELOGGERPENDINGENTITIES_H
#define KTP_MOBILELOGGERPENDINGENTITIES_H

#include <KTp/Logger/pending-logger-entities.h>


class MobileLoggerPendingEntities : public KTp::PendingLoggerEntities
{
    Q_OBJECT

public:
    explicit MobileLoggerPendingEntities(const Tp::AccountPtr &account,
                                         QObject *parent = nullptr);
};

#endif // KTP_MOBILELOGGERPENDINGENTITIES_H
