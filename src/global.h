// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include <QStringLiteral>
#include <QByteArray>
#include <TelepathyQt/SharedPtr>
#pragma once

class ChannelHandler;

constexpr auto APPLICATION_ID = "org.kde.spacebear";

#define SL QStringLiteral
#define BL QByteArrayLiteral

using ChannelHandlerPtr = Tp::SharedPtr<ChannelHandler>;
