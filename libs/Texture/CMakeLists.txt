project(Texture)
list(APPEND CMAKE_MODULE_PATH  ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
  message(STATUS "glew found")
  message(STATUS "GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR}")
  message(STATUS "GLEW_LIBRARY ${GLEW_LIBRARY}")
 else()
  message(STATUS "glew NOT found" )
endif()
message(STATUS "MODEL GLEW INCLUDE: ${GLEW_INCLUDE_DIR}")
set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
include_directories(
  ../stb_image
   ${GLEW_INCLUDE_DIR}
)

add_library( 
Texture
Texture.cpp
Texture.h
)

target_link_libraries(Texture PUBLIC stb_image)


target_include_directories(Texture PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(FILES Texture.h   DESTINATION ${CMAKE_INSTALL_PREFIX}/Texture/include)
