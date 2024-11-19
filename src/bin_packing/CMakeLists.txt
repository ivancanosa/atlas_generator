add_library(bin_packing STATIC bin_packing.cpp)

target_include_directories(bin_packing PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_library(atlas::bin_packing ALIAS bin_packing)

