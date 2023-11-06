set(spacebar_daemon_SRCS
    main.cpp
    channellogger.cpp
    ecurl.cpp
)

# generate DBus adaptor xml
qt_generate_dbus_interface(
    ${CMAKE_CURRENT_SOURCE_DIR}/channellogger.h
    org.kde.spacebar.Daemon.xml
    OPTIONS -s -m -P
)

# use generated DBus adapator xml to generate adaptor source code
qt_add_dbus_adaptor(spacebar_daemon_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.spacebar.Daemon.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/channellogger.h ChannelLogger
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})

kconfig_add_kcfg_files(spacebar-daemon ../src/settingsmanager.kcfgc GENERATE_MOC)

target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar-daemon PRIVATE
    Qt6::Concurrent
    Qt6::Core
    Qt6::Sql
    Qt6::DBus
    KF6::I18n
    KF6::KIOGui
    KF6::ConfigGui
    KF6::DBusAddons
    KF6::Notifications
    KF6::ModemManagerQt
    database
    kTelephonySupport
    macros
    mms
    CURL::libcurl
    PkgConfig::c-ares
    QCoro6::Core
)


configure_file(daemon-autostart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/daemon-autostart.desktop)

install(TARGETS spacebar-daemon DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/daemon-autostart.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR} RENAME org.kde.spacebar.daemon.desktop)
