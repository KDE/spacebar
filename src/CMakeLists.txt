set(spacebear_SRCS
    chatlistmodel.cpp
    database.cpp
    main.cpp
    contactmapper.cpp
    global.h
    resources.qrc
)

add_executable(spacebear ${spacebear_SRCS})
target_compile_definitions(spacebear PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(spacebear
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Sql
    Qt5::Widgets
    KF5::I18n
    KF5::People
    KF5::PeopleBackend
    KF5::Contacts
    TelepathyQt5::Core
)
install(TARGETS spacebear ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
