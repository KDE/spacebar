cmake_minimum_required(VERSION 3.0)

project(spacebear)

include(FeatureSummary)

################# set KDE specific information #################
find_package(ECM 0.0.8 REQUIRED NO_MODULE)
find_package(TelepathyQt5 REQUIRED)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)

################# Find dependencies #################
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Widgets Sql)
find_package(KF5 REQUIRED COMPONENTS Kirigami2 I18n People Contacts)

################# build and install #################
add_subdirectory(src)

install(PROGRAMS org.kde.spacebear.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.spacebear.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
