cmake_minimum_required(VERSION 3.10)
project(mock_key_value_store)

set(CMAKE_CXX_STANDARD 14)

add_executable(mock_key_value_store kv_store/src/main.cpp)


target_link_libraries(mock_key_value_store mock_kv_store)

enable_testing()

if(MSVC)
    target_compile_options(mock_key_value_store PUBLIC /W4)
else()
    target_compile_options(mock_key_value_store PUBLIC -Wall -Wextra -pedantic)
endif()

if (MSVC AND (MSVC_VERSION GREATER 1900))
    if (${use_control_flow_guard})
        message("Setting CONTROL FLOW GUARD")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
        SET(CMAKE_EXE_LINKER_FLAGS  "/guard:cf /DYNAMICBASE")
    endif()
    if (${use_qspectre})
        message("Setting QSPECTRE")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Qspectre")
    endif()
endif()


add_subdirectory(kv_store)
add_subdirectory(test)
add_subdirectory(applications)
