
kpackage_install_package(package org.kde.spacebar genericqml)
install(FILES package/metadata.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} RENAME org.kde.spacebar.desktop)
ecm_install_icons(ICONS 256-apps-spacebar.png DESTINATION ${ICON_INSTALL_DIR})

include_directories(${TELEPATHY_QT5_INCLUDE_DIR})

add_subdirectory(src)
# add_subdirectory(plugin)
