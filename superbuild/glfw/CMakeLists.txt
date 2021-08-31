cmake_minimum_required (VERSION 3.15) 
project(glfw)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("glfw INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_glfw_subproject(
  NAME glfw
  URL "https://github.com/ivlab/glfw.git"
  BUILD_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)