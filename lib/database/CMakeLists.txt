add_library(database STATIC database.cpp migrations.qrc)
target_compile_definitions(database PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(database
    PUBLIC Qt::Core Qt::Sql Qt::DBus macros kTelephonySupport
    FutureSQL6::FutureSQL QCoro6::Core)
target_include_directories(database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
