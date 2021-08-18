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


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()