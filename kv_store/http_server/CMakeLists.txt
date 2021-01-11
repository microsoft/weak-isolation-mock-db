cmake_minimum_required(VERSION 3.10)
project(mock_kv_store_server)

set(CMAKE_CXX_STANDARD 14)

#if(MSVC)
#    set(cpprestsdk_DIR D:/a/1/b/vcpkg/packages/cpprestsdk_x86-windows/share/cpprestsdk/)
#    set(ZLIB_LIBRARY D:/a/1/b/vcpkg/packages/zlib_x86-windows/bin/)
#    set(ZLIB_INCLUDE_DIR D:/a/1/b/vcpkg/packages/zlib_x86-windows/include/)
#    set(unofficial-brotli_DIR D:/a/1/b/vcpkg/packages/brotli_x86-windows/share/unofficial-brotli/)
#else()
#    set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
#endif()

#find_package(cpprestsdk REQUIRED)

add_executable(mock_kv_store_server start_server.cpp)

#target_link_libraries(mock_kv_store_server PRIVATE cpprestsdk::cpprest)

if(MSVC)
    target_compile_options(mock_kv_store_server PUBLIC /W4)
else()
    target_compile_options(mock_kv_store_server PUBLIC -Wall -Wextra -pedantic)
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
