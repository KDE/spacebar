# SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_library(mms STATIC mms.cpp ecurl.cpp)
kconfig_add_kcfg_files(mms ../../src/settingsmanager.kcfgc GENERATE_MOC)
target_compile_definitions(mms PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(mms PUBLIC Qt5::Core Qt5::Concurrent KF5::ConfigGui macros kTelephonySupport CURL::libcurl c-ares::cares)

if (TARGET QCoro5::Core)
    target_link_libraries(mms PUBLIC QCoro::Core)
else()
    target_link_libraries(mms PUBLIC QCoro::QCoroCore)
endif()

target_include_directories(mms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
