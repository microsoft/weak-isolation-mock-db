cmake_minimum_required(VERSION 3.10)
project(test)

include_directories(${mock_kv_store_SOURCE_DIR}/include)

file(GLOB test_files "*.cpp")
foreach(test_file ${test_files})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} mock_kv_store)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()