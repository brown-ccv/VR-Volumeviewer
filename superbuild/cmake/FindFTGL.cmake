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
	   ftgld
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FTGL
	DEFAULT_MSG
	FTGL_INCLUDE_DIR
	FTGL_LIBRARY
)

