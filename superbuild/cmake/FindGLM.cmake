message("FINDGLM CMAKE")
message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

find_path(GLM_INCLUDE_DIR
        NAMES
		glm/glm.hpp
		HINTS
        ${CMAKE_INSTALL_PREFIX}/include/
)

message(STATUS "GLM_INCLUDE_DIR  ${GLM_INCLUDE_DIR}")


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM
	DEFAULT_MSG
	GLM_INCLUDE_DIR
)

