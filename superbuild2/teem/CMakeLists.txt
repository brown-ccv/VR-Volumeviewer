cmake_minimum_required (VERSION 3.15) 
project(teem)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("teem INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_svn_subproject(
   NAME teem
   URL "https://svn.code.sf.net/p/teem/code/teem/trunk"
   BUILD_ARGS
    -DTeem_ZLIB=ON 
	-DBUILD_TESTING=OFF
)