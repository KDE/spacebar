set(spacebar_SRCS
    main.cpp
    about.cpp
    chatlistmodel.cpp
    messagemodel.cpp
    utils.cpp
    databasethread.cpp
    asyncdatabase.cpp
    channelhandler.cpp
)

set(DaemonXML ${CMAKE_CURRENT_BINARY_DIR}/../daemon/org.kde.spacebar.Daemon.xml)
qt_add_dbus_interface(spacebar_SRCS ${DaemonXML} daemoninterface)

if(QT_MAJOR_VERSION STREQUAL "5")
    qtquick_compiler_add_resources(RESOURCES resources.qrc)
else()
    qt_add_resources(RESOURCES resources.qrc)
endif()

add_executable(spacebar ${spacebar_SRCS} ${RESOURCES})

kconfig_add_kcfg_files(spacebar settingsmanager.kcfgc GENERATE_MOC)

add_dependencies(spacebar spacebar-daemon)

target_include_directories(spacebar PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(spacebar PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Sql
    Qt::Widgets
    Qt::DBus
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::I18n
    KF5::Contacts
    KF5::People
    KF5::PeopleBackend
    KF5::KIOGui
    database
    kTelephonySupport
    macros
    QCoro::Core
    QCoro::DBus
)

install(TARGETS spacebar ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
