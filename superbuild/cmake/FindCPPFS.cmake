message("FindCPPFS CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(CPPFS_INCLUDE_DIR
        NAMES
		cppfs/cppfs.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "CPPFS_INCLUDE_DIR  ${CPPFS_INCLUDE_DIR}")


find_library(CPPFS_LIBRARY
       NAMES
	   cppfs
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CPPFS
	DEFAULT_MSG
	CPPFS_INCLUDE_DIR
	CPPFS_LIBRARY
)

