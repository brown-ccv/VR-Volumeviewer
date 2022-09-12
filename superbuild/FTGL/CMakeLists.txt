cmake_minimum_required (VERSION 3.15) 
project(FTGL)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("FTGL INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")
message("FTGL CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_git_subproject(
   NAME FTGL
   URL https://github.com/ulrichard/ftgl.git
   TAG master
   BUILD_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DBUILD_SHARED_LIBS=OFF
	-DFREETYPE_INCLUDE_DIR_ft2build=${CMAKE_INSTALL_PREFIX}/include/freetype2
)