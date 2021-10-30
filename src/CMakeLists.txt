set(spacebar_SRCS
    chatlistmodel.cpp
    main.cpp
    messagemodel.cpp
    utils.cpp
    databasethread.cpp
    asyncdatabase.cpp
    channelhandler.cpp
    contactmodel.cpp
    resources.qrc
)

set(DaemonXML ${CMAKE_CURRENT_BINARY_DIR}/../daemon/org.kde.spacebar.Daemon.xml)
qt5_add_dbus_interface(spacebar_SRCS ${DaemonXML} daemoninterface)

add_executable(spacebar ${spacebar_SRCS})
target_include_directories(spacebar PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(spacebar PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Sql
    Qt5::Widgets
    Qt5::DBus
    KF5::CoreAddons
    KF5::I18n
    KF5::People
    KF5::PeopleBackend
    KF5::ModemManagerQt
    database
    kTelephonySupport
    macros
    QCoro::QCoroCore
    QCoro::QCoroDBus
)
install(TARGETS spacebar ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
