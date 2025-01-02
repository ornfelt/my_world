cmake_minimum_required(VERSION 3.28)
project(wow_world_messages_cpp LANGUAGES CXX VERSION 0.1.0)

add_library(wow_world_messages_cpp_wow_world_messages_cpp src/all.cpp src/vanilla.cpp src/tbc.cpp src/wrath.cpp src/util.cpp src/puff.c)
target_sources(wow_world_messages_cpp_wow_world_messages_cpp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include "${CMAKE_CURRENT_BINARY_DIR}"
        FILES
        include/wow_world_messages_cpp/wow_world_messages.hpp
        "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_cpp_export.h"
)
add_library(wow_world_messages_cpp::wow_world_messages_cpp ALIAS wow_world_messages_cpp_wow_world_messages_cpp)
target_compile_features(wow_world_messages_cpp_wow_world_messages_cpp PUBLIC cxx_std_11)
set_target_properties(wow_world_messages_cpp_wow_world_messages_cpp
        PROPERTIES
        CXX_EXTENSIONS OFF

        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        # CMake target is prefixed with project to avoid clashes
        OUTPUT_NAME wow_world_messages_cpp
        EXPORT_NAME wow_world_messages_cpp

        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
include(GenerateExportHeader)
generate_export_header(wow_world_messages_cpp_wow_world_messages_cpp
        BASE_NAME wow_world_messages_cpp
)

include(CMakePackageConfigHelpers)
install(TARGETS wow_world_messages_cpp_wow_world_messages_cpp EXPORT wow_world_messages_cpp FILE_SET HEADERS)
install(EXPORT wow_world_messages_cpp NAMESPACE wow_world_messages_cpp:: DESTINATION lib/cmake/wow_world_messages_cpp)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_cppConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_cppConfigVersion.cmake" DESTINATION lib/cmake/wow_world_messages_cpp)

if (BUILD_TESTING)
    add_executable(wow_world_messages_cpp_vanilla.test src/vanilla.test.cpp)
    target_link_libraries(wow_world_messages_cpp_vanilla.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_cpp_vanilla.test COMMAND wow_world_messages_cpp_vanilla.test)

    add_executable(wow_world_messages_cpp_tbc.test src/tbc.test.cpp)
    target_link_libraries(wow_world_messages_cpp_tbc.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_cpp_tbc.test COMMAND wow_world_messages_cpp_tbc.test)

    add_executable(wow_world_messages_cpp_wrath.test src/wrath.test.cpp)
    target_link_libraries(wow_world_messages_cpp_wrath.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_cpp_wrath.test COMMAND wow_world_messages_cpp_wrath.test)
endif ()
