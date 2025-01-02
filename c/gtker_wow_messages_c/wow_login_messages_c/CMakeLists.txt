cmake_minimum_required(VERSION 3.28)
project(wow_world_messages_c LANGUAGES C VERSION 0.1.0)

add_library(wow_login_messages_c_wow_login_messages_c src/login.c)
target_sources(wow_login_messages_c_wow_login_messages_c
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include "${CMAKE_CURRENT_BINARY_DIR}"
        FILES
        include/wow_login_messages/wow_login_messages.h
        include/wow_login_messages/all.h
        include/wow_login_messages/version2.h
        include/wow_login_messages/version3.h
        include/wow_login_messages/version5.h
        include/wow_login_messages/version6.h
        include/wow_login_messages/version7.h
        include/wow_login_messages/version8.h
        "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_c_export.h"
)
add_library(wow_login_messages_c::wow_login_messages_c ALIAS wow_login_messages_c_wow_login_messages_c)
target_compile_features(wow_login_messages_c_wow_login_messages_c PUBLIC c_std_90)
set_target_properties(wow_login_messages_c_wow_login_messages_c
        PROPERTIES
        C_EXTENSIONS OFF

        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        # CMake target is prefixed with project to avoid clashes
        OUTPUT_NAME wow_login_messages_c
        EXPORT_NAME wow_login_messages_c

        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
include(GenerateExportHeader)
generate_export_header(wow_login_messages_c_wow_login_messages_c
        BASE_NAME wow_login_messages_c
)

include(CMakePackageConfigHelpers)
install(TARGETS wow_login_messages_c_wow_login_messages_c EXPORT wow_login_messages_c FILE_SET HEADERS)
install(EXPORT wow_login_messages_c NAMESPACE wow_login_messages_c:: DESTINATION lib/cmake/wow_login_messages_c)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_cConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wow_login_messages_cConfigVersion.cmake" DESTINATION lib/cmake/wow_login_messages_c)

if (BUILD_TESTING)
    add_executable(wow_login_messages_c_all.test src/all.test.c)
    target_link_libraries(wow_login_messages_c_all.test PRIVATE wow_messages_test_utils)
    add_test(NAME wow_login_messages_c_all.test COMMAND wow_login_messages_c_all.test)
endif ()
