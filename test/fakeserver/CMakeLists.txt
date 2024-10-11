set(spacebar-fakeserver_SRCS
    main.cpp
    server.cpp
    messagingmocker.cpp
    modemmocker.cpp
    smsmocker.cpp
    rootmocker.cpp
    objectmanagermocker.cpp
    dbusobject.cpp
)

set(mm_messaging_dbusXML dbus/org.freedesktop.ModemManager1.Modem.Messaging.xml)
set(mm_modem_dbusXML dbus/org.freedesktop.ModemManager1.Modem.xml)
set(mm_sms_dbusXML dbus/org.freedesktop.ModemManager1.Sms.xml)
set(mm_root_dbusXML dbus/org.freedesktop.ModemManager1.xml)
set(mm_objectmanager_dbusXML dbus/org.freedesktop.DBus.ObjectManager.xml)

qt_add_dbus_adaptor(
    spacebar-fakeserver_SRCS
    ${mm_messaging_dbusXML}
    messagingmocker.h
    MessagingMocker
    messagingadaptor)

qt_add_dbus_adaptor(
    spacebar-fakeserver_SRCS
    ${mm_modem_dbusXML}
    modemmocker.h
    ModemMocker
    modemadaptor)

qt_add_dbus_adaptor(
    spacebar-fakeserver_SRCS
    ${mm_sms_dbusXML}
    smsmocker.h
    SmsMocker
    smsadaptor)

qt_add_dbus_adaptor(
    spacebar-fakeserver_SRCS
    ${mm_root_dbusXML}
    rootmocker.h
    RootMocker
    rootadaptor)

qt_add_dbus_adaptor(
    spacebar-fakeserver_SRCS
    ${mm_objectmanager_dbusXML}
    objectmanagermocker.h
    ObjectManagerMocker
    objectmanageradaptor)

add_executable(spacebar-fakeserver ${spacebar-fakeserver_SRCS})

qt_add_qml_module(spacebar-fakeserver
    URI org.kde.spacebar.fakeserver
    VERSION 1.0
)

target_link_libraries(spacebar-fakeserver PUBLIC
    Qt::Qml
    Qt::Gui
    Qt::Widgets
    Qt::Quick
    Qt::DBus
    KF6::ModemManagerQt
    KF6::CoreAddons
    KF6::DBusAddons
)

qt_target_qml_sources(spacebar-fakeserver
    QML_FILES
        qml/Main.qml
)

target_include_directories(spacebar-fakeserver PRIVATE ${CMAKE_BINARY_DIR})
install(TARGETS spacebar-fakeserver ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
