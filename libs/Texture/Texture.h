#ifndef TEXTURELOADER_H_
#define TEXTURELOADER_H_

#include <GL/glew.h>
#include <string>

class Texture
{
public:
	Texture(GLenum TextureTarget, const std::string& filename);
	~Texture();

	void Bind(GLenum TextureUnit);
	GLuint GetTextureId();

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

	

	GLuint m_texture_id;
	GLenum m_texture_target;
	const char* m_file_name;
};

#endif // FRAMEBUFFER_H_