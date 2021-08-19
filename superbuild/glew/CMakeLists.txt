cmake_minimum_required (VERSION 3.15) 
project(glew)
include(ExternalProject)
include(GNUInstallDirs)

include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("GLEW INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

#the URL parametes doesnt do anything in this case
build_glew_subproject(
  NAME glew
  URL "https://sourceforge.net/projects/glew/files/glew/snapshots/glew-20190928.zip"
)