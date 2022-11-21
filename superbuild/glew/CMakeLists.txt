cmake_minimum_required (VERSION 3.15) 
project(glew)
include(ExternalProject)
include(GNUInstallDirs)

include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("GLEW INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")
message("GLEW CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

#the URL parametes doesnt do anything in this case
build_glew_subproject(
 NAME glew
 URL "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip"
)