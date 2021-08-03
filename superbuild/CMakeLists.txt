

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/install)

include(ExternalProject)
include(GNUInstallDirs)


# Using 3.9 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.9) 

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

#---------------------- Source for this Project ----------------------

#----------------------SUPERBUILD--------------------------

project(MinVR-Volume)



set(DEFAULT_BUILD_COMMAND cmake -G "${CMAKE_GENERATOR}" --build . )


get_filename_component(INSTALL_DIR_ABSOLUTE
  ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(EXTERNAL_DIR_LOCATION ${CMAKE_BINARY_DIR})  
set(EXTERNAL_DIR_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/install/)  

include(${CMAKE_CURRENT_SOURCE_DIR}/macros.cmake)

if(WIN32)
 set(OS_MAKE_COMMAND Ninja)
elseif (UNIX)
 set(OS_MAKE_COMMAND )
endif()
  
  
build_git_subproject(
  NAME zlib
  URL "https://github.com/madler/zlib.git"
)

build_svn_subproject(
   NAME teem
   URL "https://svn.code.sf.net/p/teem/code/teem/trunk"
   BUILD_ARGS
    -DTeem_ZLIB=ON 
	-DBUILD_TESTING=OFF
  DEPENDS_ON zlib
)

build_git_subproject(
  NAME opencv
  URL "https://github.com/opencv/opencv.git"
  BUILD_ARGS
   -DBUILD_TESTS=OFF
   -DBUILD_PERF_TESTS=OFF
   -DBUILD_opencv_python_tests=OFF
)

#the URL parametes doesnt do anything in this case
build_glew_subproject(
  NAME glew
  URL "https://sourceforge.net/projects/glew/files/glew/snapshots/glew-20190928.zip"
)

#build_glfw_subproject(
#  NAME glfw
#  URL "https://github.com/ivlab/glfw.git"
#)

#build_openvr_subproject(
#  NAME openvr
#  URL "https://github.com/ivlab/openvr.git"
#  #URL "https://github.com/ValveSoftware/openvr.git"
#)


#build_git_subproject(
#  NAME minvr
#  URL "https://github.com/MinVR/MinVR.git"
#  BUILD_ARGS
#   -DWITH_PLUGIN_OPENGL=ON
#   -DWITH_PLUGIN_GLFW=ON
#   #-DWITH_PLUGIN_OPENVR=ON 
#   -DAUTOBUILD_DEPENDENCIES=OFF
#  #DEPENDS_ON glew glfw openvr
#  DEPENDS_ON glew glfw
#  
#)

#build_git_subproject(
#  NAME vr-imgui 
#  URL https://github.com/brown-ccv/VR-imgui.git
#  BUILD_ARGS
#   -DBUILD_WITH_VIRTUALKEYBOARD=OFF 
#   -DBUILD_WITH_FILEBROWSER=ON 
#   -DBUILD_WITH_EXAMPLE=ON
#   DEPENDS_ON glew minvr
#)

#build_git_subproject(
#   NAME freetype
#   URL https://gitlab.freedesktop.org/freetype/freetype.git
#   DEPENDS_ON zlib
#)

#build_git_subproject(
#   NAME CPPFSD
#   URL https://github.com/cginternals/cppfs.git
#)


message(STATUS "==== BUILDING ${PROJECT_NAME}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")

#option(WITH_TEEM "Adds support of nrrd files through teem" OFF)


set(source_files
  ${CMAKE_CURRENT_BINARY_DIR}/../src/main.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/vrapp/VolumeVisualizationApp.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/vrapp/VRVolumeApp.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/loader/LoadDataAction.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/Shader.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/Volume.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/VolumeSliceRenderer.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/VolumeSliceShader.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/VolumeRaycastRenderer.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/VolumeRaycastShader.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/UI/UIView.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/glm.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/FrameBufferObject.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/DepthTexture.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/Labels.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/render/FontHandler.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_widget.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_multichannel_widget.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/histogram.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/Vec2.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/ArcBall.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/HelperFunctions.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/CreateMovieAction.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/loader/LoadDescriptionAction.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/../src/loader/VRDataLoader.cpp
)

set(header_files
  ${CMAKE_CURRENT_BINARY_DIR}/../include/vrapp/VolumeVisualizationApp.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/vrapp/VRVolumeApp.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/LoadDataAction.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/Shader.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/Volume.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeSliceRenderer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeSliceShader.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeRaycastRenderer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeRaycastShader.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/UI/UIView.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/glm.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/FrameBufferObject.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/DepthTexture.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/Labels.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/render/FontHandler.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_widget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_multichannel_widget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/embedded_colormaps.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/histogram.h
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/Vec2.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/ArcBall.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/HelperFunctions.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/CreateMovieAction.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/LoadDescriptionAction.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/VRDataLoader.h
)





#---------------------- Find & Add Dependencies ----------------------

#set(EXTERNAL_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

ADD_DEFINITIONS(-D FTGL_LIBRARY_STATIC)

# MinVR (linked with an imported cmake target so no need to specify include dirs)
#find_package(MinVR REQUIRED)
#find_package(ImguiVR REQUIRED)
#find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
#FIND_PACKAGE(glm REQUIRED )
#FIND_PACKAGE(OpenCV REQUIRED )
#FIND_PACKAGE(FTGL REQUIRED )
#FIND_PACKAGE(Freetype REQUIRED) 
find_package(ZLIB REQUIRED)
#FIND_PACKAGE(CPPFSD REQUIRED)

include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${MINVR_INCLUDE_DIR}
  ${GLM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${FTGL_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${Freetype_INCLUDE_DIRS}
  ${CPPFSD_INCLUDE_DIRS}
)



set(LIBRARIES
	ImguiVR
	${OpenCV_LIBS}
	${GLM_LIBRARIES}
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${FTGL_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${ZLIB_LIBRARIES}
	${Freetype_LIBRARIES}
	${CPPFSD_LIBRARIES}
)

#if (UNIX)
#	set(LIBRARIES
#		${LIBRARIES}
#		stdc++fs
#	)
#endif (UNIX)

#if(WITH_TEEM)
#	FIND_PACKAGE(Teem REQUIRED )
#	include(${Teem_USE_FILE})
#	set(LIBRARIES 
#		${LIBRARIES} 
#		teem)
#	add_definitions(-DWITH_TEEM)
#	set(header_files
#		${header_files}
#		include/loader/LoadNrrdAction.h
#	)
#	set(source_files
#		${source_files}
#		src/loader/LoadNrrdAction.cpp
#	)
#endif()


#---------------------- Define the Target ----------------------


add_executable(${PROJECT_NAME} ${source_files} ${header_files})
add_dependencies(${PROJECT_NAME} zlib teem opencv)

target_link_libraries(${PROJECT_NAME}  ${ZLIB_LIBRARIES})




#file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
#foreach(_source IN ITEMS ${_source_list})
#    get_filename_component(_source_path "${_source}" PATH)
#    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
#    string(REPLACE "/" "\\" _group_path "${_group_path}")
#    source_group("${_group_path}" FILES "${_source}")
#endforeach()

