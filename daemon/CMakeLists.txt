set(spacebar_daemon_SRCS
    main.cpp
    channellogger.cpp
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})
target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar-daemon
    Qt5::Core
    Qt5::Sql
    KF5::I18n
    KF5::Notifications
    TelepathyQt5::Core
    database
    tputils
)

install(TARGETS spacebar-daemon DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
configure_file(org.freedesktop.Telepathy.Client.SpaceObserver.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceObserver.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceObserver.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES SpaceObserver.client DESTINATION ${KDE_INSTALL_DATAROOTDIR}/telepathy/clients/)
