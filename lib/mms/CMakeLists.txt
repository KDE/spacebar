# SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_library(mms STATIC mms.cpp)
target_compile_definitions(mms PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(mms PUBLIC Qt6::Core KF6::ConfigGui macros kTelephonySupport config)

target_include_directories(mms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
