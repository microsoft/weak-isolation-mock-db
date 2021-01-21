cmake_minimum_required(VERSION 3.10)
project(kv_store_apps)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -pthread)

if(NOT MSVC)
    set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
endif()


find_package(cpprestsdk REQUIRED)

# courseware
add_executable(courseware_app courseware/run_courseware.cpp utils.h utils.cpp app_config.h courseware/course.h courseware/student.h courseware/courseware.h)

# shopping_cart
add_executable(shopping_cart_app shopping_cart/run_shopping_cart.cpp utils.h utils.cpp app_config.h shopping_cart/shopping_cart.h shopping_cart/item.h shopping_cart/user.h)

# twitter
add_executable(twitter_app twitter/run_twitter.cpp utils.h utils.cpp app_config.h twitter/user.h twitter/twitter.h twitter/tweet.h)

# treiber_stack
add_executable(stack_app treiber_stack/run_stack.cpp utils.h utils.cpp app_config.h treiber_stack/treiber_stack.h)


target_link_libraries(courseware_app mock_kv_store cpprestsdk::cpprest)
target_link_libraries(shopping_cart_app mock_kv_store cpprestsdk::cpprest)
target_link_libraries(twitter_app mock_kv_store cpprestsdk::cpprest)
target_link_libraries(stack_app mock_kv_store cpprestsdk::cpprest)

if(MSVC)
    target_compile_options(courseware_app PUBLIC /W4)
    target_compile_options(shopping_cart_app PUBLIC /W4)
    target_compile_options(twitter_app PUBLIC /W4)
    target_compile_options(stack_app PUBLIC /W4)
else()
    target_compile_options(courseware_app PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(shopping_cart_app PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(twitter_app PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(stack_app PUBLIC -Wall -Wextra -pedantic)
endif()

if (MSVC AND (MSVC_VERSION GREATER 1900))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
    SET(CMAKE_EXE_LINKER_FLAGS  "/guard:cf /DYNAMICBASE")
endif()
