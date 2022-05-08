set(spacebar_SRCS
    main.cpp
    about.cpp
    chatlistmodel.cpp
    messagemodel.cpp
    utils.cpp
    databasethread.cpp
    asyncdatabase.cpp
    channelhandler.cpp
    resources.qrc
)

set(DaemonXML ${CMAKE_CURRENT_BINARY_DIR}/../daemon/org.kde.spacebar.Daemon.xml)
qt5_add_dbus_interface(spacebar_SRCS ${DaemonXML} daemoninterface)

add_executable(spacebar ${spacebar_SRCS})

kconfig_add_kcfg_files(spacebar settingsmanager.kcfgc GENERATE_MOC)

add_dependencies(spacebar spacebar-daemon)

if(Corrosion_FOUND)
    corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rs/Cargo.toml)

    # Teach cmake that this static library needs to be linked against SQLite to find all symbols
    set_property(TARGET spacebar-rust APPEND PROPERTY INTERFACE_LINK_LIBRARIES SQLite::SQLite3)
    target_include_directories(spacebar PRIVATE
        ${CMAKE_BINARY_DIR}/cargo/build/${Rust_CARGO_TARGET}/cxxbridge/spacebar-rust/src/)
endif()

target_include_directories(spacebar PRIVATE ${CMAKE_BINARY_DIR})
target_compile_definitions(spacebar PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Sql
    Qt5::Widgets
    Qt5::DBus
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::I18n
    KF5::Contacts
    KF5::People
    KF5::PeopleBackend
    KF5::ModemManagerQt
    QCoro::QCoroCore
    QCoro::QCoroDBus
    database
    kTelephonySupport
    macros
    mms
    spacebar-rust
)

if (TARGET QCoro5::Core)
    target_link_libraries(spacebar PRIVATE
        QCoro::Core
        QCoro::DBus
    )
else()
    target_link_libraries(spacebar PRIVATE
        QCoro::QCoroCore
        QCoro::QCoroDBus
    )
endif()
install(TARGETS spacebar ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
