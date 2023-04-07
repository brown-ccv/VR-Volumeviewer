message("FindTeem CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(Teem_INCLUDE_DIRS
        NAMES
		  nrrd.h
		  ten.h
		HINTS
         ${CMAKE_INSTALL_PREFIX}/include/teem
		NO_DEFAULT_PATH
)

message(STATUS "Teem_INCLUDE_DIRS  ${Teem_INCLUDE_DIRS}")


find_library(Teem_LIBRARY_DIRS
       NAMES
	   teem
	   teemd
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
	   NO_DEFAULT_PATH
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Teem
	DEFAULT_MSG
	Teem_INCLUDE_DIRS
	Teem_LIBRARY_DIRS
)
