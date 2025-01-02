cmake_minimum_required(VERSION 3.28)
project(wow_world_messages_cpp LANGUAGES CXX VERSION 0.1.0)

add_library(wow_login_messages_cpp_wow_login_messages_cpp src/login.cpp)
target_sources(wow_login_messages_cpp_wow_login_messages_cpp
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include "${CMAKE_CURRENT_BINARY_DIR}"
        FILES
        include/wow_login_messages_cpp/wow_login_messages.hpp
        include/wow_login_messages_cpp/all.hpp
        "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_cpp_export.h"
)
add_library(wow_login_messages_cpp::wow_login_messages_cpp ALIAS wow_login_messages_cpp_wow_login_messages_cpp)
target_compile_features(wow_login_messages_cpp_wow_login_messages_cpp PUBLIC cxx_std_11)
set_target_properties(wow_login_messages_cpp_wow_login_messages_cpp
        PROPERTIES
        CXX_EXTENSIONS OFF

        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        # CMake target is prefixed with project to avoid clashes
        OUTPUT_NAME wow_login_messages_cpp
        EXPORT_NAME wow_login_messages_cpp

        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
include(GenerateExportHeader)
generate_export_header(wow_login_messages_cpp_wow_login_messages_cpp
        BASE_NAME wow_login_messages_cpp
)

include(CMakePackageConfigHelpers)
install(TARGETS wow_login_messages_cpp_wow_login_messages_cpp EXPORT wow_login_messages_cpp FILE_SET HEADERS)
install(EXPORT wow_login_messages_cpp NAMESPACE wow_login_messages_cpp:: DESTINATION lib/cmake/wow_login_messages_cpp)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_cppConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_cppConfigVersion.cmake" DESTINATION lib/cmake/wow_login_messages_cpp)

if (BUILD_TESTING)
    add_executable(wow_login_messages_cpp_all.test src/all.test.cpp)
    target_link_libraries(wow_login_messages_cpp_all.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_login_messages_cpp_all.test COMMAND wow_login_messages_cpp_all.test)
endif ()
