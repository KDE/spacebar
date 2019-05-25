add_subdirectory(ktp-plugins)

set(spacebar_SRCS
    main.cpp
    )

qt5_add_resources(RESOURCES resources.qrc)
add_executable(spacebar ${spacebar_SRCS} ${RESOURCES})
target_link_libraries(spacebar Qt5::Core Qt5::Qml Qt5::Quick Qt5::Svg KF5::DBusAddons KF5::I18n)
install(TARGETS spacebar ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})


configure_file(org.freedesktop.Telepathy.Client.SpaceBar.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceBar.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceBar.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES SpaceBar.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
