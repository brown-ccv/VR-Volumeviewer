
#Using 3.19 to get a modern version of FindOpenGL.cmake
cmake_minimum_required (VERSION 3.19) 

project(VR-Volumeviewer)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME})

include(ExternalProject)
include(GNUInstallDirs)

message("CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}" CACHE PATH "default install path" FORCE )
endif()

message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_INSTALL_PREFIX})
list(APPEND CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})


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

#add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(DEFAULT_BUILD_COMMAND cmake -G "${CMAKE_GENERATOR}" . )

if(NOT CMAKE_BUILD_TYPE )
 set(CMAKE_BUILD_TYPE Debug)
endif()


message("CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

if(NOT DEBUG_SUFFIX AND CMAKE_BUILD_TYPE MATCHES "Debug")
 set(CMAKE_DEBUG_SUFFIX "d")
endif()

message("DEBUG_SUFFIX ${DEBUG_SUFFIX}")

get_filename_component(INSTALL_DIR_ABSOLUTE
  ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  
message("MAIN INSTALL_DIR_ABSOLUTE ${INSTALL_DIR_ABSOLUTE}")

set(SUPERBUILD_ROOT_PATH ${CMAKE_SOURCE_DIR})
message("==== CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/zlib/zlib")
message("xxxxxx CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
message("xxxxxx CMAKE_VERSION ${CMAKE_VERSION}")


set(MINVR_PATCH_PATH ${CMAKE_SOURCE_DIR}/../patches)
message("MINVR_PATCH_PATH ${MINVR_PATCH_PATH}")

find_package(OpenGL REQUIRED)

#GLEW
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/glew/glew)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
    ${CMAKE_SOURCE_DIR}/glew
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glew 
)

execute_process( COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glew  --config Debug )
execute_process( COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glew  --config Release ) 

ENDIF()


#glfw
#Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/glfw/glfw)
execute_process(
    COMMAND ${CMAKE_COMMAND}
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/glfw
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw
)

# Build external project
execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glfw --config ${CMAKE_BUILD_TYPE})
ENDIF()

if (WIN32)
# some windows systems are no sure they are win32, forced it to be aware of
 message("====== MINVR IS WIN 32")
else()
 message("====== MINVR IS NOT WIN 32")
endif (WIN32)


#Choreograph
# Configure external project
execute_process(
     COMMAND ${CMAKE_COMMAND}      
     -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
     -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
     -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
     -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
     ${CMAKE_SOURCE_DIR}/Choreograph
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Choreograph
 )

 
#Build external project
execute_process( COMMAND ${CMAKE_COMMAND} 
  --build ${CMAKE_BINARY_DIR}/Choreograph 
  --config ${CMAKE_BUILD_TYPE} 
)


find_package(Choreograph REQUIRED)
if(Choreograph_FOUND)
  message(STATUS "Choreograph found")
  message(STATUS "CHOREOGRAPH_INCLUDE_DIR ${CHOREOGRAPH_INCLUDE_DIR}")
  message(STATUS "CHOREOGRAPH_LIBRARY ${CHOREOGRAPH_LIBRARY}")
else()
  message(STATUS "Choreograph NOT found" )
endif()


#MinVR
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/MinVR/MinVR)
 execute_process(
     COMMAND ${CMAKE_COMMAND} 
     -DMINVR_PATCH_PATH=${MINVR_PATCH_PATH} 
     -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
     -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
     -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
     -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
     ${CMAKE_SOURCE_DIR}/MinVR
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/MinVR
 )

 
#Build external project
execute_process( COMMAND ${CMAKE_COMMAND} 
  --build ${CMAKE_BINARY_DIR}/MinVR 
  --config ${CMAKE_BUILD_TYPE} 
  
  )


ENDIF()

find_package(GLEW REQUIRED)
if(GLEW_FOUND)
  message(STATUS "glew found")
  message(STATUS "GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR}")
  message(STATUS "GLEW_LIBRARY ${GLEW_LIBRARY}")
 else()
  message(STATUS "glew NOT found" )
endif()


#glm
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/glm/glm)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/glm
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glm
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/glm --config ${CMAKE_BUILD_TYPE}
)
ENDIF()


find_package(GLM REQUIRED)
if(GLM_FOUND)
  message(STATUS "GLM found")
  message(STATUS "GLM_INCLUDE_DIR ${GLM_INCLUDE_DIR}")
 else()
  message(STATUS "GLM NOT found" )
endif()

#vr-imgui
# Configure external project
#IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/vr-imgui/vr-imgui)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE}
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DGLEW_SHARED_LIBRARY_DEBUG=${GLEW_LIBRARY}
    -DGLEW_SHARED_LIBRARY_RELEASE=${GLEW_LIBRARY}
    ${CMAKE_SOURCE_DIR}/vr-imgui
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/vr-imgui
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    --build ${CMAKE_BINARY_DIR}/vr-imgui
    --config ${CMAKE_BUILD_TYPE}
)
#ENDIF()


### ZLIB
message("==== ZLIB CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/zlib/zlib")
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/zlib/zlib)

message("++++ ZLIB CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/zlib/zlib")

message("++++ ${CMAKE_COMMAND} -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} ${CMAKE_SOURCE_DIR}/zlib WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/zlib")
 execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/zlib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/zlib
 )
 
 message("++++ COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/zlib")
 
 # Build external project
 execute_process( COMMAND ${CMAKE_COMMAND} 
 --build ${CMAKE_SOURCE_DIR}/zlib 
 --config ${CMAKE_BUILD_TYPE} 
 )
ENDIF()

#TEEM
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/teem/teem)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/teem
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/teem
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    --build ${CMAKE_BINARY_DIR}/teem
    --config ${CMAKE_BUILD_TYPE}
)
ENDIF()


#freetype
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/freetype/freetype)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/freetype
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/freetype
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    --build ${CMAKE_BINARY_DIR}/freetype 
    --config ${CMAKE_BUILD_TYPE}
)
ENDIF()

message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

#FTGL
# Configure external project
#IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/FTGL/FTGL)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/FTGL
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/FTGL
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/FTGL
)
#ENDIF()

#CCPFS
# Configure external project
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/CPPFSD/CPPFSD)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/CPPFSD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CPPFSD  
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/CPPFSD --config ${CMAKE_BUILD_TYPE}
)


ENDIF()


#opencv
# Configure external project
#IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/opencv/opencv)
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DINSTALL_DIR_ABSOLUTE=${INSTALL_DIR_ABSOLUTE} 
    -DDEFAULT_BUILD_COMMAND=${DEFAULT_BUILD_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    ${CMAKE_SOURCE_DIR}/opencv
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/opencv
)

# Build external project
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/opencv --config ${CMAKE_BUILD_TYPE}
)

#ENDIF()


find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  message(STATUS "ZLIB found")
  message(STATUS "ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIR}")
 else()
  message(STATUS "ZLIB NOT found" )
endif()

find_package(MinVR REQUIRED)
if(MinVR_FOUND)
  message(STATUS "MinVR found")
  message(STATUS "MinVR_INCLUDE_DIR ${MinVR_INCLUDE_DIR}")
 else()
  message(STATUS "MINNVR NOT found" )
endif()

find_package(ImguiVR REQUIRED)
if(IMGUIVR_FOUND)
  message(STATUS "IMGUIVR found")
  message(STATUS "IMGUIVR_INCLUDE_DIR ${IMGUIVR_INCLUDE_DIR}")
 else()
  message(STATUS "IMGUIVR NOT found" )
endif()

# find_package(OpenGL REQUIRED)
FIND_PACKAGE(FTGL REQUIRED )
if(FTGL_FOUND)
  message(STATUS "FTGL found")
  message(STATUS "FTGL_INCLUDE_DIR ${FTGL_INCLUDE_DIR}")
 else()
  message(STATUS "FTGL NOT found" )
endif()

# find_package(OpenGL REQUIRED)
FIND_PACKAGE(Freetype REQUIRED )
if(Freetype_FOUND)
  message(STATUS "Freetype found")
  message(STATUS "Freetype_INCLUDE_DIR ${Freetype_INCLUDE_DIR}")
 else()
  message(STATUS "Freetype NOT found" )
endif()

FIND_PACKAGE(CPPFS REQUIRED) 
if(CPPFS_FOUND)
  message(STATUS "CPPFS found")
  message(STATUS "CPPFS_INCLUDE_DIR ${CPPFS_INCLUDE_DIR}")
 else()
  message(STATUS "CPPFS NOT found" )
endif()


find_package(Imgui REQUIRED)
if(Imgui_FOUND)
  message(STATUS "Imgui found")
  message(STATUS "Imgui_INCLUDE_DIR ${Imgui_INCLUDE_DIR}")
 else()
  message(STATUS "Imgui NOT found" )
endif()


find_package(OpenCV CONFIG REQUIRED)
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
if(OpenCV_FOUND)
  message(STATUS "OpenCV found ${_OpenCV_LIB_PATH}")
  message(STATUS "OpenCV found ${OpenCV_INSTALL_PATH}")
  if(WIN32)
   set(OPENCV_PATH ${_OpenCV_LIB_PATH} )
   set(OPENCV_SUFIX "455" )
  elseif(APPLE)
   set(OPENCV_PATH "${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}/lib")
   set(OPENCV_PREFIX "lib" )
   unset(DEBUG_SUFFIX)
   #set(OPENCV_SUFIX "4.5.3" )
  endif()

  file(COPY "${OPENCV_PATH}/${OPENCV_PREFIX}opencv_videoio${OPENCV_SUFIX}${CMAKE_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin" )
  file(COPY "${OPENCV_PATH}/${OPENCV_PREFIX}opencv_imgcodecs${OPENCV_SUFIX}${CMAKE_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin" )
  file(COPY "${OPENCV_PATH}/${OPENCV_PREFIX}opencv_imgproc${OPENCV_SUFIX}${CMAKE_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin" )
  file(COPY "${OPENCV_PATH}/${OPENCV_PREFIX}opencv_core${OPENCV_SUFIX}${CMAKE_DEBUG_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin" )
  
endif()


message("OpenCV_LIBRARIES ${OpenCV_LIBRARIES}")
message("FTGL_LIBRARY ${FTGL_LIBRARY}")




execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DGLEW_INCLUDE_DIR=${GLEW_INCLUDE_DIR} 
    -DGLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/../libs/GLMLoader -B ${CMAKE_CURRENT_BINARY_DIR}/GLMLoader
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/GLMLoader --config ${CMAKE_BUILD_TYPE}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DGLEW_INCLUDE_DIR=${GLEW_INCLUDE_DIR} 
    -DGLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/../libs/Model -B ${CMAKE_CURRENT_BINARY_DIR}/Model 
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/Model --config ${CMAKE_BUILD_TYPE}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DGLEW_INCLUDE_DIR=${GLEW_INCLUDE_DIR} 
    -DGLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/../libs/Texture -B ${CMAKE_CURRENT_BINARY_DIR}/Texture 
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/Texture --config ${CMAKE_BUILD_TYPE}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DGLEW_INCLUDE_DIR=${GLEW_INCLUDE_DIR} 
    -DGLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ShaderProgram -B ${CMAKE_CURRENT_BINARY_DIR}/ShaderProgram 
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/ShaderProgram --config ${CMAKE_BUILD_TYPE}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
    -DGLEW_INCLUDE_DIR=${GLEW_INCLUDE_DIR} 
    -DGLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/../libs/stb_image -B ${CMAKE_CURRENT_BINARY_DIR}/stb_image 
)
execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/stb_image --config ${CMAKE_BUILD_TYPE}
)


 find_package(GLMLoader REQUIRED)
 if(GLMLoader_FOUND)
 message(STATUS "GLMLoader found")
 message(STATUS "GLMLoader_LIBRARY ${GLMLoader_LIBRARY}")
else()
 message(STATUS "GLMLoader NOT found" )
endif()

find_package(Texture REQUIRED)
if(Texture_FOUND)
message(STATUS "Texture found")
message(STATUS "Texture ${Texture_LIBRARY}")
else()
message(STATUS "Texture NOT found" )
endif()

find_package(ShaderProgram REQUIRED)
if(ShaderProgram_FOUND)
message(STATUS "ShaderProgram found")
message(STATUS "ShaderProgram_LIBRARY ${ShaderProgram_LIBRARY}")
else()
message(STATUS "ShaderProgram NOT found" )
endif()


find_package(Model REQUIRED)
if(Model_FOUND)
message(STATUS "Model found")
message(STATUS "Model_INCLUDE_DIR ${Model_INCLUDE_DIR}")
message(STATUS "Model_LIBRARY ${Model_LIBRARY}")
else()
message(STATUS "Model NOT found" )
endif()

find_package(Stb_image REQUIRED)
if(Stb_image_FOUND)
message(STATUS "Stb_image found")
message(STATUS "Stb_image_LIBRARY ${Stb_image_LIBRARY}")
else()
message(STATUS "Model NOT found" )
endif()

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
 ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/LabelManager.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/render/FontHandler.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_widget.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_multichannel_widget.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/histogram.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/Vec2.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/ArcBallCamera.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/HelperFunctions.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/CreateMovieAction.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/interaction/Simulation.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/loader/LoadDescriptionAction.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/../src/loader/VRDataLoader.cpp
)

set(header_files
 ${CMAKE_CURRENT_BINARY_DIR}/../include/vrapp/VolumeVisualizationApp.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/vrapp/VRVolumeApp.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/LoadDataAction.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/Shader.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/Volume.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeRenderer.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeSliceRenderer.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeSliceShader.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeRaycastRenderer.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/VolumeRaycastShader.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/UI/UIView.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/glm.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/FrameBufferObject.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/DepthTexture.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/LabelManager.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/render/FontHandler.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_widget.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/transfer_function_multichannel_widget.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/embedded_colormaps.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/histogram.h
 ${CMAKE_CURRENT_BINARY_DIR}/../libs/UIHelpers/Vec2.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/ArcBallCamera.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/HelperFunctions.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/CreateMovieAction.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/PointOfInterests.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/interaction/Simulation.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/LoadDescriptionAction.h
 ${CMAKE_CURRENT_BINARY_DIR}/../include/loader/VRDataLoader.h
 
)


 include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${MinVR_INCLUDE_DIR}
  ${GLM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR}
  ${FTGL_INCLUDE_DIR}
  ${Freetype_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
  ${CPPFS_INCLUDE_DIRS}
  ${Imgui_INCLUDE_DIR}
  ${IMGUIVR_INCLUDE_DIR}
  ${CHOREOGRAPH_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../libs/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
  ${Model_INCLUDE_DIR}
  ${GLMLoader_INCLUDE_DIR}
  ${Texture_INCLUDE_DIR}
  ${ShaderProgram_INCLUDE_DIR}
)


set(LIBRARIES
  ${OPENGL_LIBRARY}
  ${CHOREOGRAPH_LIBRARY}
	${OpenCV_LIBS}
	${GLM_LIBRARIES}
	${GLEW_LIBRARY}
  ${MinVR_LIBRARY}
	${OPENGL_LIBRARY}
	${FTGL_LIBRARY}
	${FREETYPE_LIBRARY}
	${ZLIB_LIBRARY}
	${Freetype_LIBRARY}
	${CPPFS_LIBRARY}
  ${IMGUIVR_LIBRARY}
  ${OpenCV_LIBRARIES}
	${Stb_image_LIBRARY}
	${GLMLoader_LIBRARY}
	${Model_LIBRARY}
  ${Texture_LIBRARY}
  ${ShaderProgram_LIBRARY}
)

message(STATUS "LIBRARIES ${LIBRARIES}")

message(STATUS "header_files ${header_files}")


 ADD_DEFINITIONS(-DFTGL_LIBRARY_STATIC)

 if (WIN32)
 # some windows systems are no sure they are win32, forced it to be aware of
 ADD_DEFINITIONS(-DWIN32 -D_WINDOWS)
endif (WIN32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}/bin)


add_executable(${PROJECT_NAME} ${source_files} ${header_files})
 
target_link_libraries(${PROJECT_NAME}  ${LIBRARIES})

file(COPY "${CMAKE_SOURCE_DIR}/../shaders" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin")
file(COPY "${CMAKE_SOURCE_DIR}/../fonts" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin")
file(COPY "${CMAKE_SOURCE_DIR}/../observers" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin")
file(COPY "${CMAKE_SOURCE_DIR}/../colormaps" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin")
message(STATUS "COPY RESOURCES FOLDER")
file(COPY "${CMAKE_SOURCE_DIR}/../Resources" DESTINATION "${INSTALL_DIR_ABSOLUTE}/bin")

enable_testing()

add_test(NAME ExistsTest COMMAND ${CMAKE_BINARY_DIR}/install_${CMAKE_SYSTEM_NAME}/bin/${PROJECT_NAME} -t)
set_tests_properties(
	ExistsTest
	PROPERTIES PASS_REGULAR_EXPRESSION "VR-Volumeviewer exists"
)
