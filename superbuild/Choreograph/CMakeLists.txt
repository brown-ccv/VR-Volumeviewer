cmake_minimum_required (VERSION 3.15) 
project(Choreograph)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("${CMAKE_PROJECT_NAME} INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_choerograph_subproject(
  NAME Choreograph 
  URL https://github.com/brown-ccv/Choreograph.git
  BUILD_ARGS
	-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)
