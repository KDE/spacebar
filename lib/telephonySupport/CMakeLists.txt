add_library(kTelephonySupport STATIC phonenumberutils.cpp avatarimageprovider.cpp contactphonenumbermapper.cpp)
target_compile_definitions(kTelephonySupport PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(kTelephonySupport PUBLIC
    Qt5::Core
    Qt5::Quick
    KF5::People
    PhoneNumber::PhoneNumber
    macros)
target_include_directories(kTelephonySupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
