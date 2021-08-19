message("FindModel CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")


find_path(Model_INCLUDE_DIR
        NAMES
		Model.h
		HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../libs/Model
)

message(STATUS "Model_INCLUDE_DIR  ${Model_INCLUDE_DIR}")

find_library(Model_LIBRARY
       NAMES
	   Model
	   HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/Model
)  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Model
	DEFAULT_MSG
	Model_INCLUDE_DIR
	Model_LIBRARY
)
