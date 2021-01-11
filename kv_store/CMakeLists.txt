cmake_minimum_required(VERSION 3.10)
project(mock_kv_store)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -pthread)

add_library(mock_kv_store src/main.cpp include/kv_store.h include/transaction.h include/key_not_found_exception.h include/operation_response.h include/operation_param.h include/consistency_checker.h include/read_response_selector.h include/consistency_exception.h include/operation.h)

add_subdirectory(http_server)

if(MSVC)
    target_compile_options(mock_kv_store PUBLIC /W4)
else()
    target_compile_options(mock_kv_store PUBLIC -Wall -Wextra -pedantic)
endif()

if (MSVC AND (MSVC_VERSION GREATER 1900))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
    SET(CMAKE_EXE_LINKER_FLAGS  "/guard:cf /DYNAMICBASE")
endif()
