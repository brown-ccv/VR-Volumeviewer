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
	   opencv_
	   opencv
	   opencv_core
	   HINTS
        ${CMAKE_INSTALL_PREFIX}/lib/
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCV
	DEFAULT_MSG
	OpenCV_INCLUDE_DIR
	OpenCV_LIBRARY
)
