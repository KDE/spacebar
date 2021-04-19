set(spacebar_daemon_SRCS
    main.cpp
    channellogger.cpp
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})
target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar-daemon
    Qt5::Core
    Qt5::Sql
    Qt5::DBus
    KF5::I18n
    KF5::Notifications
    PkgConfig::QOfono
    database
    kTelephonySupport
    macros
)

install(TARGETS spacebar-daemon DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
