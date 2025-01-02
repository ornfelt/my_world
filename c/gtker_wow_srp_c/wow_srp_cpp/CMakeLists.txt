add_library(wow_srp_cpp src/wow_srp_cpp.cpp)
target_sources(wow_srp_cpp PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
        include ${CMAKE_CURRENT_BINARY_DIR}
        FILES
        include/wow_srp_cpp/client.hpp
        include/wow_srp_cpp/server.hpp
        include/wow_srp_cpp/vanilla.hpp
        include/wow_srp_cpp/wow_srp.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/wow_srp_cpp_export.h
)
add_library(WowSrp::wow_srp_cpp ALIAS wow_srp_cpp)

target_compile_features(wow_srp_cpp PUBLIC cxx_std_17)

target_link_libraries(wow_srp_cpp PRIVATE WowSrp::wow_srp)
set_target_properties(wow_srp_cpp PROPERTIES INSTALL_RPATH $ORIGIN)

include(GenerateExportHeader)
generate_export_header(wow_srp_cpp)
