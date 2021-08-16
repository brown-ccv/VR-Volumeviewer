message("FindOpenCV CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(OpenCV_INCLUDE_DIR
        NAMES
		opencv2/opencv.hpp
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "OpenCV_INCLUDE_DIR  ${OpenCV_INCLUDE_DIR}")


find_library(OpenCV_LIBRARY
       NAMES
	   cppfs
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCV
	DEFAULT_MSG
	OpenCV_INCLUDE_DIR
	OpenCV_LIBRARY
)


#if(glew_FOUND)
#	set(glew_INCLUDE_DIR "${GAINPUT_INCLUDE_DIR}" ${_deps_includes})
#	set(glew_LIBRARY "${GAINPUT_LIBRARY}" ${_deps_libs})
#
#endif()