cmake_minimum_required(VERSION 3.10)
project(Example VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

include(FetchContent)
include(cmake/extern.cmake)
include(CTest)
include(GoogleTest)
include(GNUInstallDirs)
enable_testing()

add_subdirectory(bin_packing)
add_subdirectory(test)

add_executable(atlas main.cpp)

target_link_libraries(atlas PRIVATE
    nlohmann_json::nlohmann_json bin_packing stb::stb argparse::argparse)

# Install the target
install(TARGETS atlas
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
