project(my_glew)
include(ExternalProject)
include(${CMAKE_CURRENT_SOURCE_DIR}/macros.cmake)

build_glew_subproject(
   NAME glew
   URL "https://sourceforge.net/projects/glew/files/glew/snapshots/glew-20190928.zip"
)