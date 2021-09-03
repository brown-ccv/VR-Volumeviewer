message("FINDMINVR CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(MinVR_INCLUDE_DIR
        NAMES
		api/MinVR.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/MinVR-2.0
)

message(STATUS "MinVR_INCLUDE_DIR  ${MinVR_INCLUDE_DIR}")


find_library(MinVR_LIBRARY
       NAMES
	   MinVR
	   MinVRd
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/MinVR-2.0/
)  

message(STATUS "MinVR_LIBRARY  ${MinVR_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MinVR
	DEFAULT_MSG
	MinVR_INCLUDE_DIR
	MinVR_LIBRARY
)

