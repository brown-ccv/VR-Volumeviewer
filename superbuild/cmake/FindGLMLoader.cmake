message("FindGLMLoader CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")


find_path(GLMLoader_INCLUDE_DIR
        NAMES
		Model.h
		VertexBuffer.h
		HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../libs/GLMLoader
)

message(STATUS "GLMLoader_INCLUDE_DIR  ${GLMLoader_INCLUDE_DIR}")

find_library(GLMLoader_LIBRARY
       NAMES
	   GLMLoader
	   HINTS
       ${CMAKE_CURRENT_SOURCE_DIR}/GLMLoader
	   ${CMAKE_CURRENT_SOURCE_DIR}/GLMLoader/Debug
	   ${CMAKE_CURRENT_SOURCE_DIR}/GLMLoader/Release
)  


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLMLoader
	DEFAULT_MSG
	GLMLoader_INCLUDE_DIR
	GLMLoader_LIBRARY
)