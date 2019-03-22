

list(APPEND CMAKE_PREFIX_PATH ../../build/install)



# Using 3.9 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.9) 

# Dependencies that are auto-downloaded, built, and installed for you will go here.  So, you 
# may use this even if not planning to install this particular project on your system.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

list(APPEND CMAKE_MODULE_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_INSTALL_PREFIX})


set(CMAKE_CXX_STANDARD 11)
    if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)


#---------------------- Source for this Project ----------------------

project(MinVR-Volume)

message(STATUS "==== BUILDING ${PROJECT_NAME}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")



set(source_files
  main.cpp
  VolumeVisualizationApp.cpp
  Data.cpp
  LoadDataAction.cpp
  Shader.cpp
  Volume.cpp
  VolumeSliceRender.cpp
  VolumeSliceRenderShader.cpp
  glm.cpp
)

set(header_files
  VolumeVisualizationApp.h
  Data.h
  LoadDataAction.h
  Shader.h
  Volume.h
  VolumeSliceRender.h
  VolumeSliceRenderShader.h
  glm.h
)


#---------------------- Find & Add Dependencies ----------------------

set(EXTERNAL_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# MinVR (linked with an imported cmake target so no need to specify include dirs)
find_package(MinVR REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED )
FIND_PACKAGE(glm REQUIRED )

set(LIBRARIES
	${OpenCV_LIBS}
	${GLM_LIBRARIES}
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
)

include_directories(
	  ${OPENGL_INCLUDE_DIR}
	  ${GLEW_INCLUDE_DIRS}
	  ${OpenCV_INCLUDE_DIRS}
      ${GLM_INCLUDE_DIRS}
)

#---------------------- Define the Target ----------------------

add_executable(${PROJECT_NAME} ${source_files} ${header_files})

target_link_libraries(${PROJECT_NAME} PUBLIC MinVR::MinVR ${LIBRARIES})

