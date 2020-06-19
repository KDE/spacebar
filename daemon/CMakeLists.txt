set(spacebar_daemon_SRCS
    ../src/database.cpp
    main.cpp
    channellogger.cpp
    ../src/global.h
)

add_executable(spacebar-daemon ${spacebar_daemon_SRCS})
target_compile_definitions(spacebar-daemon PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_include_directories(spacebar-daemon PRIVATE "../src/")
target_link_libraries(spacebar-daemon
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Sql
    KF5::I18n
    TelepathyQt5::Core
)
install(TARGETS spacebar-daemon ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
