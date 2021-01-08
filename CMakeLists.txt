cmake_minimum_required(VERSION 3.10)
project(mock_key_value_store)

set(CMAKE_CXX_STANDARD 14)

add_executable(mock_key_value_store kv_store/src/main.cpp)

if(MSVC)
  target_compile_options(mock_key_value_store PRIVATE /W4)
else()
  target_compile_options(mock_key_value_store PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(mock_key_value_store mock_kv_store)

enable_testing()

add_subdirectory(kv_store)
add_subdirectory(test)
add_subdirectory(applications)
