add_subdirectory(mobile-logger)
add_subdirectory(ktp-logger-plugin)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.kde.ktp-mobile.presence/metadata.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop
               @ONLY)

install(DIRECTORY org.kde.ktp-mobile.presence DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids PATTERN metadata.desktop.cmake EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.ktp-mobile.presence)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-org.kde.ktp-mobile.presence.desktop)
