message("FindFreetype CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(Freetype_INCLUDE_DIR
        NAMES
		ft2build.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/freetype2
)

message(STATUS "Freetype_INCLUDE_DIR  ${Freetype_INCLUDE_DIR}")


find_library(Freetype_LIBRARY
       NAMES
	   freetype
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype
	DEFAULT_MSG
	Freetype_INCLUDE_DIR
	Freetype_LIBRARY
)
