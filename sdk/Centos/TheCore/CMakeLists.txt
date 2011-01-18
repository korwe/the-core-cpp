cmake_minimum_required(VERSION 2.6)
project(THE_CORE)

link_directories(/usr/local/lib)

add_subdirectory(pugixml)
add_subdirectory(messaging)

include_directories (pugixml) 
include_directories (messaging) 

link_directories (pugixml)
link_directories (messaging)

add_executable(TestClient TestClient.cpp)
target_link_libraries(TestClient core_messaging)

add_executable(TestServer TestServer.cpp)
target_link_libraries(TestServer core_messaging)
