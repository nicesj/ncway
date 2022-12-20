cmake_minimum_required(VERSION 3.4.1)
project(ncway CXX)

include(FindPkgConfig)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 0)
set(CMAKE_POISITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(server)
add_subdirectory(client)
