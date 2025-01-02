cmake_minimum_required(VERSION 3.28)
project(wow_messages LANGUAGES C CXX)

if (BUILD_TESTING)
    include(CTest)

    add_library(wow_messages_test_utils STATIC test_utils/test_utils.c)
    target_include_directories(wow_messages_test_utils PUBLIC test_utils)
    target_link_libraries(wow_messages_test_utils
            PUBLIC
            wow_login_messages_c::wow_login_messages_c
            wow_world_messages_c::wow_world_messages_c
            wow_login_messages_cpp::wow_login_messages_cpp
            wow_world_messages_cpp::wow_world_messages_cpp
    )

    if (BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif ()
endif ()

add_subdirectory(wow_login_messages_c)
add_subdirectory(wow_world_messages_c)
add_subdirectory(wow_login_messages_cpp)
add_subdirectory(wow_world_messages_cpp)
