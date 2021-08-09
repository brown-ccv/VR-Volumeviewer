# Using 3.15 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.15)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/install)

include(ExternalProject)
include(GNUInstallDirs)




# Dependencies that are auto-downloaded, built, and installed for you will go here.  So, you 
# may use this even if not planning to install this particular project on your system.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

list(APPEND CMAKE_MODULE_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_INSTALL_PREFIX})

set(CMAKE_CXX_STANDARD 17)
    if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)
add_definitions(-D_HAS_STD_BYTE=0)

if (UNIX)
	add_compile_options(-lpthread)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
	SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17 -pthread")
endif (UNIX)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

project(MinVR-Volume)

set(DEFAULT_BUILD_COMMAND cmake -G "${CMAKE_GENERATOR}" --build . )


get_filename_component(INSTALL_DIR_ABSOLUTE
  ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/install/)  

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)

if(WIN32)
 set(OS_MAKE_COMMAND Ninja)
elseif (UNIX)
 set(OS_MAKE_COMMAND )
endif()



# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}/zlib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/zlib
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/zlib
)