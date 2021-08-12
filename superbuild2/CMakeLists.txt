
# Using 3.9 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.15) 


set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/install)

include(ExternalProject)
include(GNUInstallDirs)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

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

project(VR-Volumeviewer)

set(DEFAULT_BUILD_COMMAND cmake -G "${CMAKE_GENERATOR}" --build . )


get_filename_component(INSTALL_DIR_ABSOLUTE
  ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  
message("MAIN INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")


### ZLIB
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/zlib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/zlib
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/zlib
)

#GLEW
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/glew
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glew
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glew
)

#TEEM
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/teem
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/teem
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/teem
)

#glfw
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/glfw
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glfw
)

#freetype
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/freetype
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/freetype
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/freetype
)

#FTGL
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/FTGL
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/FTGL
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/FTGL
)

#CCPFS
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/CPPFSD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CPPFSD
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/CPPFSD
)

#opencv
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/opencv
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/opencv
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/opencv
)

#MinVR
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/MinVR
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/MinVR
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/MinVR
)

#vr-imgui
# Configure external project
execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/vr-imgui
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/vr-imgui
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/vr-imgui
)

find_package(GLEW CONFIG REQUIRED)
if(GLEW_FOUND)
  message(STATUS "glew found")
  message(STATUS "GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIRS}")
 else()
  message(STATUS "glew NOT found" )
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libs/GLMLoader ${CMAKE_CURRENT_BINARY_DIR}/GLMLoader)