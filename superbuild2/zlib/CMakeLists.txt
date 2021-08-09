project(my_zlib)
include(ExternalProject)
include(${CMAKE_SOURCE_DIR}/../cmake/macros.cmake)

build_git_subproject(
   NAME zlib
   URL "https://github.com/madler/zlib.git"
)