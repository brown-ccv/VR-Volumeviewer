# STB_IMAGE
add_library(
stb_image
stb_image.cpp
stb_image.h
)

target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


install(FILES stb_image.h DESTINATION ${CMAKE_INSTALL_PREFIX}/stb_image/include)