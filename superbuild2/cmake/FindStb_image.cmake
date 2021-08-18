message("FindStb_image CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

find_library(Stb_image_LIBRARY
       NAMES
	   stb_image
	   HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Stb_image
	DEFAULT_MSG
	Stb_image_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()