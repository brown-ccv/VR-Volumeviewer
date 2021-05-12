#include "Texture.h"
#include "stb_image.h"
#include <cassert>
#include <iostream>

Texture::Texture(GLenum TextureTarget, const std::string& filename) : m_texture_target(TextureTarget)
, m_file_name(filename.c_str())
{
	m_texture_id = Load2DTexture();
}
	
Texture::~Texture()
{
}

void Texture::Bind(GLenum TextureUnit)
{
	glActiveTexture(GL_TEXTURE0 + TextureUnit);
	glBindTexture(m_texture_target, m_texture_id);
}

GLuint Texture::GetTextureId()
{
	return m_texture_id;
}

int Texture::Load2DTexture()
{
	

	int width = 0;
	int height = 0;
	int bbp = 0;
	//pointer to the image data
	unsigned char * bits(0);

	//retrieve the image data
	//if this somehow one of these failed (they shouldn't), return failure
	GLuint textureID = LoadTexture(m_file_name, width, height, &bits, bbp);
	if (textureID == -1)
	{
		assert(false && "Image failed to load 2");
	}

	
	return textureID;
}

int Texture::LoadTexture(const std::string & fileName, 
	int & width, int & height, unsigned char ** data, int& bbp)
{
	

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	//int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(1);
	*data = stbi_load(fileName.c_str(), &width, &height, &bbp, 0);

	if (data)
	{
		if (bbp == 3)
		{
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, *data);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,0, GL_RGB, GL_UNSIGNED_BYTE, *data);
		}
		else if (bbp == 4)
		{
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, *data);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,0, GL_RGBA, GL_UNSIGNED_BYTE, *data);
		}
		
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return -1;
	}
	glBindTexture(GL_TEXTURE_2D,0);
	stbi_image_free(*data);

	return texture;
}
