message("FindOpenVR CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(OPENVR_INCLUDE_DIR
        NAMES
		openvr/openvr.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "OPENVR_INCLUDE_DIR  ${OPENVR_INCLUDE_DIR}")


find_library(OPENVR_LIBRARY
       NAMES
	   openvr_api64
	   openvr_api
	   HINTS
		${CMAKE_INSTALL_PREFIX}/lib/
		${CMAKE_INSTALL_PREFIX}/lib64/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVR
	DEFAULT_MSG
	OPENVR_INCLUDE_DIR
	OPENVR_LIBRARY
)
