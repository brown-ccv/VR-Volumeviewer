#ifndef TEXTURELOADER_H_
#define TEXTURELOADER_H_

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <string>
#include <vector>

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& filename);
	~Texture();

	void Bind();
	GLuint GetTextureId();
	void UnBind();

	int LoadTexture(const std::string& fileName, int& width, int& height, unsigned char ** data, int& bbp);

private:

	struct image
	{
		image() : imageWidth(0), imageHeight(0), imageData(0) {};
		image(int w, int h, unsigned char* data)
			: imageWidth(w), imageHeight(h), imageData(data) {};
		int imageWidth;
		int imageHeight;
		unsigned char* imageData;
	};

	int Load2DTexture();
	int Load3DTexture(const std::vector<std::string>& paths);
	

	GLuint m_texture_id;
	GLenum m_texture_target;
	const char* m_file_name;
};

#endif // FRAMEBUFFER_H_