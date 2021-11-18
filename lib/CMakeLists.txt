add_library(macros STATIC global.h)
target_include_directories(macros PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(telephonySupport)
add_subdirectory(database)
add_subdirectory(mms)
