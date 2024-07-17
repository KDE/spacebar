# SPDX-FileCopyrightText: 2024 Jonah Brüchert <jbb@kaidan.im>
# SPDX-License-Identifier: BSD-2-Clause

add_library(config STATIC)

kconfig_add_kcfg_files(config settingsmanager.kcfgc GENERATE_MOC)

target_link_libraries(config PUBLIC Qt6::Core KF6::ConfigGui KF6::ConfigCore)
