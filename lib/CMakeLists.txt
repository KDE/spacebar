add_library(database STATIC database.cpp global.h)
target_compile_definitions(database PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(database PUBLIC Qt5::Core Qt5::Sql)
target_include_directories(database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tputils STATIC accountutils.cpp)
target_compile_definitions(tputils PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(tputils PUBLIC Qt5::Core TelepathyQt5::Core)
target_include_directories(tputils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
