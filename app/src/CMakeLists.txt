
set(spacebar_SRCS
    main.cpp
)

add_executable(spacebar ${spacebar_SRCS})
target_compile_definitions(spacebar PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(spacebar
 Qt5::Gui
 Qt5::Quick
#  Qt5::Widgets
 KF5::Declarative
 KF5::I18n
 KF5::Package
#  KF5::QuickAddons
 KF5::DBusAddons
#  KF5::Notifications
#  KF5::Activities
#  ${TELEPATHY_QT5_LIBRARIES}
)

install(TARGETS spacebar ${INSTALL_TARGETS_DEFAULT_ARGS})

configure_file(org.freedesktop.Telepathy.Client.SpaceBar.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceBar.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.SpaceBar.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES SpaceBar.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
