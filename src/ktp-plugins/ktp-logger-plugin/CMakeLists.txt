add_library (ktploggerplugin_mobile MODULE
     mobile-logger.cpp
     mobile-logger-pending-dates.cpp
     mobile-logger-pending-entities.cpp
     mobile-logger-pending-logs.cpp
#      mobile-logger-pending-search.cpp
)

target_link_libraries (ktploggerplugin_mobile
    KTp::CommonInternals
    KTp::Logger
    Qt5::Sql
    Qt5::Core
    KF5::Service
    TelepathyQt5::Core
)
kde_target_enable_exceptions(ktploggerplugin_mobile PRIVATE)

# Install:
install (TARGETS ktploggerplugin_mobile
         DESTINATION ${PLUGIN_INSTALL_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ktploggerplugin_mobile.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_mobile.desktop
               @ONLY)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/ktploggerplugin_mobile.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}
)

