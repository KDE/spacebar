set(spacebarqmlplugin_SRCS
    mainlogmodel.cpp
    spacebarqmlplugin.cpp
    )

add_library(spacebarqmlplugin SHARED ${spacebarqmlplugin_SRCS})
target_link_libraries(spacebarqmlplugin Qt5::Core
                                        Qt5::Qml
                                        Qt5::Sql
                                        )

install(TARGETS spacebarqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/spacebar)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/spacebar)
