message("FindGLMLoader CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

find_path(ShaderProgram_INCLUDE_DIR
        NAMES
		ShaderProgram.h
		HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ShaderProgram
)

message(STATUS "ShaderProgram_INCLUDE_DIR  ${ShaderProgram_INCLUDE_DIR}")

find_library(ShaderProgram_LIBRARY
       NAMES
	   ShaderProgram
	   HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/ShaderProgram
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ShaderProgram
	DEFAULT_MSG
	ShaderProgram_INCLUDE_DIR
	ShaderProgram_LIBRARY
)

