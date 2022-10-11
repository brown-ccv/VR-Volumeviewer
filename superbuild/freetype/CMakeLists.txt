cmake_minimum_required (VERSION 3.15) 
project(freetype)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("freetype INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_git_subproject(
   NAME freetype
   URL https://gitlab.freedesktop.org/freetype/freetype.git
   TAG master
   BUILD_ARGS
   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
   -DFT_DISABLE_ZLIB=TRUE
   -DFT_DISABLE_BZIP2=TRUE
   -DFT_DISABLE_PNG=TRUE
   -DFT_DISABLE_HARFBUZZ=TRUE
   -DFT_DISABLE_BROTLI=TRUE
)