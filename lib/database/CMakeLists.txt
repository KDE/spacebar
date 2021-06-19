add_library(database STATIC database.cpp)
target_compile_definitions(database PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(database PUBLIC Qt5::Core Qt5::Sql macros kTelephonySupport)
target_include_directories(database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
