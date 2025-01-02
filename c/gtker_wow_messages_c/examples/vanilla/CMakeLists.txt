add_executable(wow_messages_vanilla src/vanilla.main.c)
target_link_libraries(wow_messages_vanilla
        PRIVATE
        wow_login_messages_c::wow_login_messages_c
        wow_world_messages_c::wow_world_messages_c
)
set_target_properties(wow_messages_vanilla
        PROPERTIES
        C_EXTENSIONS ON
)
target_compile_features(wow_messages_vanilla PUBLIC c_std_90)

include(FetchContent)
FetchContent_Declare(
        wow_srp
        GIT_REPOSITORY https://github.com/gtker/wow_srp_c.git
        GIT_TAG b451f89414f1304fda85e51940fa9e0fdfd63ef3
)
FetchContent_MakeAvailable(wow_srp)

target_link_libraries(wow_messages_vanilla
        PRIVATE
        WowSrp::wow_srp
        # Windows sockets
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

add_executable(wow_messages_vanilla_cpp src/vanilla.main.cpp)
target_link_libraries(wow_messages_vanilla_cpp
        PRIVATE
        wow_login_messages_cpp::wow_login_messages_cpp
        wow_world_messages_cpp::wow_world_messages_cpp
)
target_compile_features(wow_messages_vanilla_cpp PUBLIC cxx_std_11)

target_link_libraries(wow_messages_vanilla_cpp
        PRIVATE
        WowSrp::wow_srp_cpp
        # Windows sockets
        $<$<PLATFORM_ID:Windows>:ws2_32>
)
