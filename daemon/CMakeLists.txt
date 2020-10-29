set(spacebar_daemon_SRCS
    main.cpp
    channellogger.cpp
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})
target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebar-daemon
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Sql
    KF5::I18n
    TelepathyQt5::Core
    database
)
install(TARGETS spacebar-daemon ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
