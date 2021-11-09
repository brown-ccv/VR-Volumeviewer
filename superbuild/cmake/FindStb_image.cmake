message("FindStb_image CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

find_library(Stb_image_LIBRARY
       NAMES
	   stb_image
	   HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image
		${CMAKE_CURRENT_SOURCE_DIR}/stb_image/Debug
		${CMAKE_CURRENT_SOURCE_DIR}/stb_image/Release
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Stb_image
	DEFAULT_MSG
	Stb_image_LIBRARY
)
