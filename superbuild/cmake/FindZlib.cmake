message("FINDZLIB CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(ZLIB_INCLUDE_DIR
        NAMES
		zlib.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "ZLIB_INCLUDE_DIR  ${ZLIB_INCLUDE_DIR}")


find_library(ZLIB_LIBRARY
       NAMES
	   zlib
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB
	DEFAULT_MSG
	ZLIB_INCLUDE_DIR
	ZLIB_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()