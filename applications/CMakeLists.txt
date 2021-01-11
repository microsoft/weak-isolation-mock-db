cmake_minimum_required(VERSION 3.10)
project(kv_store_apps)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -pthread)

if(MSVC)
    set(cpprestsdk_DIR /a/1/b/vcpkg/packages/cpprestsdk_x64-windows/share/cpprestsdk/)
else()
    set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
endif()


find_package(cpprestsdk REQUIRED)

# courseware
add_executable(kv_store_apps courseware/run_courseware.cpp utils.h utils.cpp app_config.h courseware/course.h courseware/student.h courseware/courseware.h)

# shopping_cart
#add_executable(kv_store_apps shopping_cart/run_shopping_cart.cpp utils.h utils.cpp app_config.h shopping_cart/shopping_cart.h shopping_cart/item.h shopping_cart/user.h)

# twitter
#add_executable(kv_store_apps twitter/run_twitter.cpp utils.h utils.cpp app_config.h twitter/user.h twitter/twitter.h twitter/tweet.h)

# treiber_stack
#add_executable(kv_store_apps treiber_stack/run_stack.cpp utils.h utils.cpp app_config.h treiber_stack/treiber_stack.h)


target_link_libraries(kv_store_apps mock_kv_store cpprestsdk::cpprest)

if(MSVC)
    target_compile_options(kv_store_apps PUBLIC /W4)
else()
    target_compile_options(kv_store_apps PUBLIC -Wall -Wextra -pedantic)
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

