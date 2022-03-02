

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

#---------------------- Source for this Project ----------------------

project(MinVR-Volume)

message(STATUS "==== BUILDING ${PROJECT_NAME}")
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")

option(WITH_TEEM "Adds support of nrrd files through teem" OFF)


set(source_files
  src/main.cpp
  src/vrapp/VolumeVisualizationApp.cpp
  src/vrapp/VRVolumeApp.cpp
  src/loader/LoadDataAction.cpp
  src/render/Shader.cpp
  src/render/Volume.cpp
  src/render/VolumeSliceRenderer.cpp
  src/render/VolumeSliceShader.cpp
  src/render/VolumeRaycastRenderer.cpp
  src/render/VolumeRaycastShader.cpp
  src/UI/UIView.cpp
  src/UI/UIDoubleClickedTableCell.cpp
  libs/glm.cpp
  src/render/FrameBufferObject.cpp
  src/render/DepthTexture.cpp
  src/interaction/Labels.cpp
  src/render/FontHandler.cpp
  libs/UIHelpers/transfer_function_widget.cpp
  libs/UIHelpers/transfer_function_multichannel_widget.cpp
  libs/UIHelpers/histogram.cpp
  libs/UIHelpers/Vec2.cpp
  src/interaction/ArcBall.cpp
  src/interaction/HelperFunctions.cpp
  src/interaction/CreateMovieAction.cpp
  src/loader/LoadDescriptionAction.cpp
  src/loader/VRDataLoader.cpp
)

set(header_files
  include/vrapp/VolumeVisualizationApp.h
  include/vrapp/VRVolumeApp.h
  include/common/common.h
  include/loader/LoadDataAction.h
  include/render/Shader.h
  include/render/Volume.h
  include/render/VolumeSliceRenderer.h
  include/render/VolumeSliceShader.h
  include/render/VolumeRaycastRenderer.h
  include/render/VolumeRaycastShader.h
  include/UI/UIView.h
  include/UI/UIDoubleClickedTableCell.h
  libs/glm.h
  include/render/FrameBufferObject.h
  include/render/DepthTexture.h
  include/interaction/Labels.h
  include/render/FontHandler.h
  libs/UIHelpers/transfer_function_widget.h
  libs/UIHelpers/transfer_function_multichannel_widget.h
  libs/UIHelpers/embedded_colormaps.h
  libs/UIHelpers/histogram.h
  libs/UIHelpers/Vec2.h
  include/interaction/ArcBall.h
  include/interaction/HelperFunctions.h
  include/interaction/CreateMovieAction.h
  include/loader/LoadDescriptionAction.h
  include/loader/VRDataLoader.h
)


#---------------------- Find & Add Dependencies ----------------------

set(EXTERNAL_CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

ADD_DEFINITIONS(-D FTGL_LIBRARY_STATIC)

# MinVR (linked with an imported cmake target so no need to specify include dirs)
find_package(MinVR REQUIRED)
find_package(ImguiVR REQUIRED)
find_package(OpenGL REQUIRED)

FIND_PACKAGE(GLM REQUIRED )
if (GLM_FOUND)
    message(STATUS "GLM_INCLUDE_DIR ${GLM_INCLUDE_DIR}")
    include_directories(${GLM_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "GLM not found")
endif ()


FIND_PACKAGE(OpenCV REQUIRED )
FIND_PACKAGE(FTGL REQUIRED )
FIND_PACKAGE(Freetype REQUIRED) 

find_library (CPPFSD_LIB cppfsd 
              /usr/local/lib/
			  ${CMAKE_PREFIX_PATH}/libs
			  )

find_path(CPPFSD_INCLUDE 
           NAMES cpp/cppfs.h cpp/FilePath.h
            HINTS  /usr/local/include
	    )

if(${CPPFSD_LIB} STREQUAL "")
 message( FATAL_ERROR  "CPPFSD_LIB not found" )
endif()

if(${CPPFSD_INCLUDE} STREQUAL "")
 message( FATAL_ERROR  "CPPFSD_INCLUDE not found" )
endif()


include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${MINVR_INCLUDE_DIR}
  ${GLM_INCLUDE_DIRS}
  
  ${FTGL_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
  ${CPPFSD_INCLUDE}
  include/
  libs/
)

if (NOT APPLE)
FIND_PACKAGE(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

endif()



set(LIBRARIES
	ImguiVR
	${OpenCV_LIBS}
	${GLM_LIBRARIES}
	
	${OPENGL_LIBRARY}
	${FTGL_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${CPPFSD_LIB}
	stb_image
	GLMLoader
	Model
	Texture
	ShaderProgram
	
)


message(STATUS "cppfs_INCLUDE_DIR ${cppfs_INCLUDE_DIRS}")
message(STATUS "cppfs_LIBRARY ${cppfs_LIBRARIES}")


if(WITH_TEEM)
	FIND_PACKAGE(Teem REQUIRED )
	include(${Teem_USE_FILE})
	set(LIBRARIES 
		${LIBRARIES} 
		teem)
	add_definitions(-DWITH_TEEM)
	set(header_files
		${header_files}
		include/loader/LoadNrrdAction.h
	)
	set(source_files
		${source_files}
		src/loader/LoadNrrdAction.cpp
	)
endif()

add_subdirectory(libs/GLMLoader)
add_subdirectory(libs/Model)
add_subdirectory(libs/Texture)
add_subdirectory(libs/ShaderProgram)
add_subdirectory(libs/stb_image)
#add_subdirectory(libs/cppfs)
# POSSIBLE FIX ON RECURSIVE INCLUDE
#add_subdirectory(src)

#---------------------- Define the Target ----------------------

add_executable(${PROJECT_NAME} ${source_files} ${header_files})

target_link_libraries(${PROJECT_NAME} PUBLIC MinVR::MinVR ${LIBRARIES})

if (NOT APPLE)

target_link_libraries(${PROJECT_NAME} GLEW::GLEW ${GLEW_LIBRARY})

endif()

file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/colormaps DESTINATION ${CMAKE_CURRENT_BINARY_DIR})