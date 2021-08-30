cmake_minimum_required (VERSION 3.15) 
project(vr-imgui)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("${CMAKE_PROJECT_NAME} INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  


build_git_subproject(
  NAME vr-imgui 
  URL https://github.com/brown-ccv/VR-imgui.git
  BUILD_ARGS
   -DBUILD_WITH_VIRTUALKEYBOARD=OFF 
   -DBUILD_WITH_FILEBROWSER=ON 
   -DBUILD_WITH_EXAMPLE=ON
   -DGLEW_SHARED_LIBRARY_DEBUG=${CMAKE_INSTALL_PREFIX}/lib/glew32${CMAKE_STATIC_LIBRARY_SUFFIX}
   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)