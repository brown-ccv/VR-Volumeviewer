message(STATUS "GLMLoader SOURCE_DIR ${CMAKE_SOURCE_DIR}")
project(GLMLoader)
list(APPEND CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
  message(STATUS "glew found")
  message(STATUS "GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR}")
  message(STATUS "GLEW_LIBRARY ${GLEW_LIBRARY}")
 else()
  message(STATUS "glew NOT found" )
endif()
message(STATUS "GLMLOADER GLEW INCLUDE: ${GLEW_INCLUDE_DIR}")
set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_CXX_STANDARD 11)
include_directories(
  ../Model
   ../Texture
  ../ShaderProgram
  ${GLEW_INCLUDE_DIR}
)

add_library( 
GLMLoader
GLMLoader.cpp
GLMLoader.h
VertexBuffer.cpp
VertexBuffer.h
)

target_include_directories(GLMLoader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" )


install(FILES GLMLoader.h VertexBuffer.h   DESTINATION ${CMAKE_INSTALL_PREFIX}/GLMLoader/include)
