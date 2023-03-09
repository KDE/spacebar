add_library(kTelephonySupport STATIC
    avatarimageprovider.cpp
    contactphonenumbermapper.cpp
    modemcontroller.cpp
    phonenumber.cpp
    phonenumberlist.cpp)
target_compile_definitions(kTelephonySupport PRIVATE -DQT_NO_CAST_FROM_ASCII)
target_link_libraries(kTelephonySupport PUBLIC
    Qt::Core
    Qt::Quick
    KF${QT_MAJOR_VERSION}::People
    KF${QT_MAJOR_VERSION}::PeopleBackend
    KF${QT_MAJOR_VERSION}::ModemManagerQt
    PhoneNumber::PhoneNumber
    macros)
target_include_directories(kTelephonySupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
