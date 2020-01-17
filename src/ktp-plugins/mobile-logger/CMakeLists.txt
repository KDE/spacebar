set(ktp-mobile-logger_SRCS
        main.cpp
        channel-watcher.cpp
        text-channel-observer.cpp
)

add_executable(ktp-mobile-logger ${ktp-mobile-logger_SRCS})

target_link_libraries(ktp-mobile-logger
            ${TELEPATHY_QT5_LIBRARIES}
            KF5::People
            KTp::CommonInternals
            KTp::Logger
            Qt5::DBus
            Qt5::Core
            Qt5::Sql)

target_include_directories(ktp-mobile-logger PUBLIC ${TELEPATHY_QT5_INCLUDE_DIR}) # TODO: Remove when TelepathyQt exports include paths properly


install(TARGETS ktp-mobile-logger DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(org.freedesktop.Telepathy.Client.KTp.Mobile.Logger.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Mobile.Logger.service)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.Mobile.Logger.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KTp.Mobile.Logger.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)
