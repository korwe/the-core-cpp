cmake_minimum_required(VERSION 2.6)
project(THE_CORE)

include_directories(../pugixml) 

link_directories(../pugixml)
link_directories(/usr/local/lib)

add_library(core_messaging SHARED CoreMessage.cpp CoreMessageSerializer.cpp Messaging.cpp)
target_link_libraries(core_messaging pugixml boost_date_time qpidclient)

add_executable(MessageTest MessageTest.cpp)
add_library(boost_unit_test_framework STATIC IMPORTED)
set_property(TARGET boost_unit_test_framework PROPERTY IMPORTED_LOCATION /usr/lib/libboost_unit_test_framework.a)
target_link_libraries(MessageTest core_messaging boost_unit_test_framework)
