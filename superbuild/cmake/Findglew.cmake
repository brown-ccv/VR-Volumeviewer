message("FINDGLEW CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(glew_INCLUDE_DIR
        NAMES
		GL/glew.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "GLEW_INCLUDE_DIR  ${glew_INCLUDE_DIR}")


find_library(glew_LIBRARY
       NAMES
	   glew32
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glew
	DEFAULT_MSG
	glew_INCLUDE_DIR
	glew_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()