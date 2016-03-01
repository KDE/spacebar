project(ktp-mobile)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


find_package (KF5 REQUIRED COMPONENTS Package Declarative DBusAddons I18n Service)
# Archive Sonnet WidgetsAddons Service Emoticons
#                                       KIO KCMUtils NotifyConfig Notifications I18n
#                                       WebKit WindowSystem XmlGui ItemViews TextWidgets
#                                       IconThemes DBusAddons)
find_package (Qt5 REQUIRED COMPONENTS Core Qml Quick Network Xml DBus Sql Gui)
find_package (KTp REQUIRED)

# find_package (KF5People)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMInstallIcons)

# set_package_properties(KF5People PROPERTIES DESCRIPTION "KDE Contacts aggregation"
#                        TYPE OPTIONAL
# )

add_subdirectory(mobile-logger)
add_subdirectory(ktp-logger-plugin)
add_subdirectory(app)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/org.kde.ktp-mobile.presence/metadata.desktop.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop
               @ONLY)

install(DIRECTORY org.kde.ktp-mobile.presence DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids PATTERN metadata.desktop.cmake EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.ktp-mobile.presence)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ktp-mobile.presence/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-org.kde.ktp-mobile.presence.desktop)
