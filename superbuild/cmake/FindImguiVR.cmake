message("FindImguiVR CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(IMGUIVR_INCLUDE_DIR
        NAMES
		VRMenuHandler.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "IMGUIVR_INCLUDE_DIR  ${IMGUIVR_INCLUDE_DIR}")


find_library(IMGUIVR_LIBRARY
       NAMES
	   ImguiVR
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IMGUIVR
	DEFAULT_MSG
	IMGUIVR_INCLUDE_DIR
	IMGUIVR_LIBRARY
)
