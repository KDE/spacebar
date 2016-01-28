

set(spacebar_SRCS
    main.cpp
)

add_executable(spacebar ${spacebar_SRCS})
target_compile_definitions(spacebar PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

#find_package(ActiveApp REQUIRED)

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

# install(TARGETS spacebar ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(FILES plasma_dialer.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})
