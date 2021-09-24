set(spacebar_daemon_SRCS
    main.cpp
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})
target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar-daemon
    Qt5::Core
    Qt5::Sql
    Qt5::DBus
    KF5::I18n
    KF5::CoreAddons
    KF5::Notifications
    KF5::ModemManagerQt
    database
    kTelephonySupport
    macros
)


configure_file(daemon-autostart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/daemon-autostart.desktop)

install(TARGETS spacebar-daemon DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/daemon-autostart.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR} RENAME org.kde.spacebar.daemon.desktop)
