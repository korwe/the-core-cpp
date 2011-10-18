cmake_minimum_required(VERSION 2.6)
project(THE_CORE)

link_directories(/usr/local/lib)

add_subdirectory(pugixml)
add_subdirectory(messaging)

include_directories (pugixml) 
include_directories (messaging) 

link_directories (pugixml)
link_directories (messaging)

add_executable(CoreClientMonitor ClientMonitor.cpp )
target_link_libraries(CoreClientMonitor core_messaging pugixml)

add_executable(CoreServiceMonitor ServiceMonitor.cpp)
target_link_libraries(CoreServiceMonitor core_messaging pugixml)

add_executable(CoreStart CoreStart.cpp)
target_link_libraries(CoreStart boost_program_options)

add_executable(TestClient TestClient.cpp)
target_link_libraries(TestClient core_messaging)

add_executable(TestServer TestServer.cpp)
target_link_libraries(TestServer core_messaging)

#add_executable(CoreTest CoreTest.cpp)
#add_library(boost_unit_test_framework STATIC IMPORTED)
#set_property(TARGET boost_unit_test_framework PROPERTY IMPORTED_LOCATION /usr/lib/libboost_unit_test_framework.a)
#target_link_libraries(CoreTest core_messaging boost_unit_test_framework)
