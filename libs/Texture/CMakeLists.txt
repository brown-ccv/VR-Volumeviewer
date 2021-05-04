add_library( 
Texture
Texture.cpp
Texture.h
)

target_link_libraries(Texture PUBLIC stb_image)


target_include_directories(Texture PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

install(FILES Texture.h   DESTINATION ${CMAKE_INSTALL_PREFIX}/Texture/include)
