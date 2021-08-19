message("FINDGLEW CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(GLEW_INCLUDE_DIR
        NAMES
		GL/glew.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "GLEW_INCLUDE_DIR  ${GLEW_INCLUDE_DIR}")


find_library(GLEW_LIBRARY
       NAMES
	   glew32
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW
	DEFAULT_MSG
	GLEW_INCLUDE_DIR
	GLEW_LIBRARY
)
