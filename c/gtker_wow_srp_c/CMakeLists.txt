cmake_minimum_required(VERSION 3.23)
project(WowSrp VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.5
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH Cargo.toml)

target_sources(wow_srp
        INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}
        FILES
        include/wow_srp/client.h
        include/wow_srp/server.h
        include/wow_srp/vanilla.h
        include/wow_srp/tbc.h
        include/wow_srp/wrath.h
        include/wow_srp/wow_srp.h
        ${CMAKE_CURRENT_BINARY_DIR}/wow_srp_export.h
)
add_library(WowSrp::wow_srp ALIAS wow_srp)

add_subdirectory(wow_srp_cpp)

install(TARGETS wow_srp wow_srp_cpp
        EXPORT WowSrpTargets
        FILE_SET HEADERS
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/WowSrpConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)

if(BUILD_SHARED_LIBS)
    if(WIN32 OR CYGWIN)
        set(WOW_SRP_DEFINE_IMPORT "__declspec(dllimport)")
    elseif(GNUCC)
        set(WOW_SRP_DEFINE_IMPORT "__attribute__((visibility(\"default\")))")
    endif()
else()
    set(WOW_SRP_DEFINE_IMPORT "")
endif()

target_compile_definitions(wow_srp INTERFACE "WOW_SRP_EXPORT=${WOW_SRP_DEFINE_IMPORT}")

if (BUILD_SHARED_LIBS)
    # Export the actual shared library
    install(IMPORTED_RUNTIME_ARTIFACTS wow_srp-shared LIBRARY)
    set(STATIC_LIBRARY_INIT "")
    set(SHARED_LIBRARY_INIT
            "
add_library(wow_srp-shared SHARED IMPORTED)
set_target_properties(wow_srp-shared
        PROPERTIES
        IMPORTED_LOCATION \${PACKAGE_PREFIX_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}wow_srp${CMAKE_SHARED_LIBRARY_SUFFIX}
        IMPORTED_SONAME libwow_srp.so
)
"
    )
else ()
    set(STATIC_LIBRARY_INIT
            "
add_library(wow_srp-static STATIC IMPORTED)
set_target_properties(wow_srp-static
        PROPERTIES
        IMPORTED_LOCATION \${PACKAGE_PREFIX_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}wow_srp${CMAKE_STATIC_LIBRARY_SUFFIX}
)
"
    )
    set(SHARED_LIBRARY_INIT "")
    target_link_libraries(wow_srp INTERFACE $<$<PLATFORM_ID:Windows>:Bcrypt>)
    install(FILES $<TARGET_PROPERTY:wow_srp-static,IMPORTED_LOCATION> DESTINATION lib)
endif ()

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/WowSrpConfig.cmake"
        INSTALL_DESTINATION lib/cmake/WowSrp
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/WowSrpConfig.cmake"
        ${CMAKE_CURRENT_BINARY_DIR}/WowSrpConfigVersion.cmake
        DESTINATION lib/cmake/WowSrp
)
install(EXPORT WowSrpTargets
        NAMESPACE WowSrp::
        DESTINATION lib/cmake/WowSrp
)
