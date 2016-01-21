set(ktp-mobile-logger_SRCS
        main.cpp
        channel-watcher.cpp
        text-channel-observer.cpp
)

add_executable(ktp-mobile-logger ${ktp-mobile-logger_SRCS})

target_link_libraries(ktp-mobile-logger
            ${TELEPATHY_QT5_LIBRARIES}
            KTp::CommonInternals
            KTp::Logger
            Qt5::Core
            Qt5::Sql)

target_include_directories(ktp-mobile-logger PUBLIC ${TELEPATHY_QT5_INCLUDE_DIR}) # TODO: Remove when TelepathyQt exports include paths properly


# install(TARGETS ktp-log-viewer DESTINATION ${BIN_INSTALL_DIR})
# install(FILES org.kde.ktplogviewer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
# install(FILES log-viewer.rc DESTINATION  ${DATA_INSTALL_DIR}/ktp-log-viewer/)
