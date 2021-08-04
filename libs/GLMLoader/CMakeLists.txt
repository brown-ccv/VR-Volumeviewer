message(STATUS "GLMLOADER GLEW INCLUDE: ${glew_INCLUDE_DIR}")

include_directories(
  ../Model
   ../Texture
  ../ShaderProgram
  ${glew_INCLUDE_DIR}
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
