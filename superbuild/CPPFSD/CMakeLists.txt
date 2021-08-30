cmake_minimum_required (VERSION 3.15) 
project(CPPFSD)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("CPPFSD INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/bin)  

build_CPPFSD_subproject(
   NAME CPPFSD
   URL https://github.com/cginternals/cppfs.git
   BUILD_ARGS
   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)