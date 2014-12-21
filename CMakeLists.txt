cmake_minimum_required (VERSION 2.6)
project (xban)

set (Tutorial_VERSION_MAJOR 0)
set (Tutorial_VERSION_MINOR 8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(INCLUDE_INSTALL_DIR)
else()
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
endif()
if(LIB_INSTALL_DIR)
else()
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()
if(BIN_INSTALL_DIR)
else()
set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

add_subdirectory (libtban)
add_subdirectory (tbancontrol)
