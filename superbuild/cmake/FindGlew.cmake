message("FINDGLEW CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(GLEW_INCLUDE_DIR
        NAMES
		GL/glew.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "GLEW_INCLUDE_DIR  ${GLEW_INCLUDE_DIR}")

message(STATUS "CMAKE_INSTALL_PREFIX_LIB  ${CMAKE_INSTALL_PREFIX}/lib/")

find_library(GLEW_LIBRARY
       NAMES
	   glew32
	   glew32d
	   GLEW
	   GLEW.2.2.0
	   GLEW.2.2
	   PATHS
	   ${CMAKE_INSTALL_PREFIX}/lib
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW
	DEFAULT_MSG
	GLEW_INCLUDE_DIR
	GLEW_LIBRARY
)
