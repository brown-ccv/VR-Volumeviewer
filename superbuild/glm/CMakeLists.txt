cmake_minimum_required (VERSION 3.15) 
project(glm)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("glm INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_glm_subproject(
  NAME glm
  URL "https://github.com/g-truc/glm"
  BUILD_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)