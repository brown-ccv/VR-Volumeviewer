message("FindFTGL CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(FTGL_INCLUDE_DIR
        NAMES
		FTGL/ftgl.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "FTGL_INCLUDE_DIR  ${FTGL_INCLUDE_DIR}")


find_library(FTGL_LIBRARY
       NAMES
	   ftgl
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FTGL
	DEFAULT_MSG
	FTGL_INCLUDE_DIR
	FTGL_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()