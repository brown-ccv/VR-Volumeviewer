message("FindTexture CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

find_path(Texture_INCLUDE_DIR
        NAMES
		Texture.h
		HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../libs/Texture
)

message(STATUS "Texture_INCLUDE_DIR  ${Texture_INCLUDE_DIR}")

find_library(Texture_LIBRARY
       NAMES
	   Texture
	   HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/Texture
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Texture
	DEFAULT_MSG
	Texture_INCLUDE_DIR
	Texture_LIBRARY
)

