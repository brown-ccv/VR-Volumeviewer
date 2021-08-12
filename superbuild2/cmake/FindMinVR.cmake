message("FINDMINVR CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(MINVR_INCLUDE_DIR
        NAMES
		api/MinVR.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/MinVR-2.0
)

message(STATUS "MINVR_INCLUDE_DIR  ${MINVR_INCLUDE_DIR}")


find_library(MINVR_LIBRARY
       NAMES
	   MinVR
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/MinVR-2.0/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MINVR
	DEFAULT_MSG
	MINVR_INCLUDE_DIR
	MINVR_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()