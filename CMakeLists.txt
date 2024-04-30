cmake_minimum_required(VERSION 3.16.0)

set(KF_MIN_VERSION "6.0.0")
set(QT_MIN_VERSION "6.6.0")

set(PROJECT_VERSION "24.05.0")
project(spacebar VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

################# set KDE specific information #################
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
find_package(PkgConfig REQUIRED)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(KDEClangFormat)
include(KDEGitCommitHooks)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX SPACEBAR
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

################# Find dependencies #################
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Concurrent Gui Qml Quick QuickControls2 Widgets Sql)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami I18n Contacts People Notifications Config CoreAddons DBusAddons ModemManagerQt KIO WindowSystem)
find_package(KF6KirigamiAddons 1.0 REQUIRED)
find_package(PhoneNumber COMPONENTS PhoneNumber REQUIRED)
find_package(CURL REQUIRED)
pkg_search_module(c-ares libcares REQUIRED IMPORTED_TARGET)
find_package(FutureSQL6 REQUIRED)
find_package(QCoro6 COMPONENTS Core DBus REQUIRED)

kde_enable_exceptions()
qcoro_enable_coroutines()

################# build and install #################
add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(src)

ki18n_install(po)

install(PROGRAMS org.kde.spacebar.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.spacebar.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.spacebar.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES spacebar.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
