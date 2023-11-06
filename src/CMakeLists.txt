set(spacebar_SRCS
    main.cpp
    about.cpp
    chatlistmodel.cpp
    messagemodel.cpp
    utils.cpp
    channelhandler.cpp
)

set(DaemonXML ${CMAKE_CURRENT_BINARY_DIR}/../daemon/org.kde.spacebar.Daemon.xml)
qt_add_dbus_interface(spacebar_SRCS ${DaemonXML} daemoninterface)

qt_add_resources(RESOURCES resources.qrc)

add_executable(spacebar ${spacebar_SRCS} ${RESOURCES})

kconfig_add_kcfg_files(spacebar settingsmanager.kcfgc GENERATE_MOC)

add_dependencies(spacebar spacebar-daemon)

target_include_directories(spacebar PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(spacebar PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Widgets
    Qt6::DBus
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::Contacts
    KF6::People
    KF6::PeopleBackend
    KF6::KIOGui
    KF6::WindowSystem
    database
    kTelephonySupport
    macros
    QCoro::Core
    QCoro::DBus
)

install(TARGETS spacebar ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
