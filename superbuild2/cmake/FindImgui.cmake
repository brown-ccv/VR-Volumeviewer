message("FindImgui CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

unset(${Imgui_INCLUDE_DIR} CACHE )

find_path(Imgui_INCLUDE_DIR
        NAMES
		imgui.h
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/imgui/
)

message(STATUS "Imgui_INCLUDE_DIR  ${Imgui_INCLUDE_DIR}")


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Imgui
	DEFAULT_MSG
	Imgui_INCLUDE_DIR
)
