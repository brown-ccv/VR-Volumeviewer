#include "Texture.h"
#include "stb_image.h"
#include <cassert>
#include <iostream>

Texture::Texture(GLenum TextureTarget, const std::string &filename) : m_texture_target(TextureTarget), m_file_name(filename.c_str())
{
	m_texture_id = Load2DTexture();
}

Texture::~Texture()
{
}

void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_texture_target, m_texture_id);
}

GLuint Texture::GetTextureId()
{
	return m_texture_id;
}

void Texture::UnBind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_texture_target, 0);
}

int Texture::Load2DTexture()
{

	int width = 0;
	int height = 0;
	int bbp = 0;
	// pointer to the image data
	unsigned char *bits(0);

	// retrieve the image data
	// if this somehow one of these failed (they shouldn't), return failure
	GLuint textureID = LoadTexture(m_file_name, width, height, &bits, bbp);
	if (textureID == -1)
	{
		assert(false && "Image failed to load 2");
	}

	return textureID;
}

int Texture::Load3DTexture(const std::vector<std::string> &paths)
{
	if (paths.size() == 0)
	{
		assert(false && "no paths to load from");
	}

	GLsizei width, height, depth = paths.size();

	std::vector<image> formatedImages(paths.size());

	// load and format each image
	for (int i = 0; i < paths.size(); ++i)
	{
		BYTE *bits(0);
		int bbp = 0;
		if (!LoadTexture(paths[i], width, height, &bits, bbp))
		{
			assert(false && "Image failed to load 2");
		}
		image img(width, height, bits);
		formatedImages[i] = img;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	// Create storage for the texture. (100 layers of 1x1 texels)
	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
				   1,			  // No mipmaps as textures are 1x1
				   GL_RGBA8,	  // Internal format
				   width, height, // width,height
				   depth		  // Number of layers
	);

	for (unsigned int i = 0; i < formatedImages.size(); ++i)
	{
		// Specify i-essim image
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
						0,																// Mipmap number
						0, 0, i,														// xoffset, yoffset, zoffset
						formatedImages[i].imageWidth, formatedImages[i].imageHeight, 1, // width, height, depth
						GL_RGB,															// format
						GL_UNSIGNED_BYTE,												// type
						formatedImages[i].imageData);									// pointer to data
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	// Return the ID of the texture we just created
	return textureID;
}

int Texture::LoadTexture(const std::string &fileName,
						 int &width, int &height, unsigned char **data, int &bbp)
{

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(1);
	*data = stbi_load(fileName.c_str(), &width, &height, &bbp, 0);

	if (data)
	{
		if (bbp == 3)
		{			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, *data);
		}
		else if (bbp == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, *data);
		}

		
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return -1;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(*data);

	return texture;
}
