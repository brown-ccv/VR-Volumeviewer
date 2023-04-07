cmake_minimum_required (VERSION 3.15) 
project(opencv)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("opencv INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_git_subproject(
  NAME openvr
  URL https://github.com/ValveSoftware/openvr
  TAG master
  BUILD_ARGS
   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
   -DBUILD_SHARED=ON
)
