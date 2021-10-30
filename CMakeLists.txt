cmake_minimum_required(VERSION 3.16.0)

set(KF5_MIN_VERSION "5.83.0")
set(QT_MIN_VERSION "5.15.0")

project(spacebar)
set(PROJECT_VERSION "21.08")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

################# set KDE specific information #################
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
find_package(PkgConfig REQUIRED)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SPACEBAR
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

################# Find dependencies #################
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Sql Xml)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n People Notifications Config CoreAddons DBusAddons ModemManagerQt)
find_package(PhoneNumber COMPONENTS PhoneNumber REQUIRED)
find_package(QCoro REQUIRED)

qcoro_enable_coroutines()

################# build and install #################
add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(src)

install(PROGRAMS org.kde.spacebar.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.spacebar.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.spacebar.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES spacebar.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
