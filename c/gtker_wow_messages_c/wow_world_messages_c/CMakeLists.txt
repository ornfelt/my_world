cmake_minimum_required(VERSION 3.28)
project(wow_world_messages_c LANGUAGES C VERSION 0.1.0)

add_library(wow_world_messages_c_wow_world_messages_c src/vanilla.c src/tbc.c src/wrath.c src/all.c src/util.c)
target_sources(wow_world_messages_c_wow_world_messages_c
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include "${CMAKE_CURRENT_BINARY_DIR}"
        FILES
        include/wow_world_messages/wow_world_messages.h
        "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_c_export.h"
)
add_library(wow_world_messages_c::wow_world_messages_c ALIAS wow_world_messages_c_wow_world_messages_c)
target_compile_features(wow_world_messages_c_wow_world_messages_c PUBLIC c_std_90)
set_target_properties(wow_world_messages_c_wow_world_messages_c
        PROPERTIES
        C_EXTENSIONS OFF

        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        # CMake target is prefixed with project to avoid clashes
        OUTPUT_NAME wow_world_messages_c
        EXPORT_NAME wow_world_messages_c

        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
include(GenerateExportHeader)
generate_export_header(wow_world_messages_c_wow_world_messages_c
        BASE_NAME wow_world_messages_c
)

include(CMakePackageConfigHelpers)
install(TARGETS wow_world_messages_c_wow_world_messages_c EXPORT wow_world_messages_c FILE_SET HEADERS)
install(EXPORT wow_world_messages_c NAMESPACE wow_world_messages_c:: DESTINATION lib/cmake/wow_world_messages_c)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_cConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wow_world_messages_cConfigVersion.cmake" DESTINATION lib/cmake/wow_world_messages_c)

if (BUILD_TESTING)
    add_executable(wow_world_messages_c_vanilla.test src/vanilla.test.c)
    target_link_libraries(wow_world_messages_c_vanilla.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_c_vanilla.test COMMAND wow_world_messages_c_vanilla.test)

    add_executable(wow_world_messages_c_tbc.test src/tbc.test.c)
    target_link_libraries(wow_world_messages_c_tbc.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_c_tbc.test COMMAND wow_world_messages_c_tbc.test)

    add_executable(wow_world_messages_c_wrath.test src/wrath.test.c)
    target_link_libraries(wow_world_messages_c_wrath.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_world_messages_c_wrath.test COMMAND wow_world_messages_c_wrath.test)
endif ()