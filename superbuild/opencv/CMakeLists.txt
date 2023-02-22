cmake_minimum_required (VERSION 3.15) 
project(opencv)
include(ExternalProject)
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/../macros.cmake)

message("opencv INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/../install/)  

build_git_subproject(
  NAME opencv
  URL https://github.com/opencv/opencv.git
  TAG 4.5.5
  BUILD_ARGS
   -DBUILD_TESTS=OFF
   -DBUILD_PERF_TESTS=OFF
   -DBUILD_opencv_python_bindings_generator=OFF
   -DBUILD_opencv_python_tests=OFF
   -DBUILD_OPENEXR=OFF
   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
   -DWITH_WIN32UI=OFF
   -DWITH_VTK=OFF
   -DBUILD_ITT=OFF
   -DBUILD_JAVA=OFF
   -DBUILD_opencv_calib3d=OFF
   -DBUILD_opencv_dnn=OFF
   -DBUILD_opencv_features2d=OFF
   -DBUILD_opencv_flann=OFF
   -DBUILD_opencv_gapi=OFF
   -DBUILD_opencv_highgui=ON
   -DBUILD_WEBP=OFF
   -DWITH_WEBP=OFF
   
)
