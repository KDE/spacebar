set(spacebar_SRCS
    chatlistmodel.cpp
    main.cpp
    messagemodel.cpp
    utils.cpp
    databasethread.cpp
    asyncdatabase.cpp
    channelhandler.cpp
    resources.qrc
)

add_executable(spacebar ${spacebar_SRCS})
target_compile_definitions(spacebar PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Sql
    Qt5::Widgets
    Qt5::DBus
    KF5::I18n
    KF5::People
    KF5::PeopleBackend
    KF5::Contacts
    PkgConfig::QOfono
    database
    kTelephonySupport
    macros
)
install(TARGETS spacebar ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
