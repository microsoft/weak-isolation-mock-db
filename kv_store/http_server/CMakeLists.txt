cmake_minimum_required(VERSION 3.10)
project(mock_kv_store_server)

set(CMAKE_CXX_STANDARD 14)
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(cpprestsdk REQUIRED)

add_executable(mock_kv_store_server start_server.cpp)

target_link_libraries(mock_kv_store_server PRIVATE cpprestsdk::cpprest)