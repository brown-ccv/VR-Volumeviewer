include_directories(
  ${glew_INCLUDE_DIR}
)
add_library( 
ShaderProgram
ShaderProgram.cpp
ShaderProgram.h

)

target_include_directories(ShaderProgram PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


install(FILES ShaderProgram.h   DESTINATION ${CMAKE_INSTALL_PREFIX}/ShaderProgram/include)
