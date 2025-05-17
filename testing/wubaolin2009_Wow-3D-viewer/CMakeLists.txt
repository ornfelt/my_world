cmake_minimum_required(VERSION 3.10)
project(wow_3d LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include all .cpp and .h files in the current directory
file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

# Create the executable
add_executable(wow_3d ${SOURCES})

# Set include directory for DirectX SDK
include_directories("C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")

# Link directories for DirectX SDK
link_directories("C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

# Link against DirectX 9 libraries
target_link_libraries(wow_3d d3d9.lib d3dx9.lib)

# Fix linker subsystem
set_target_properties(wow_3d PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

