#include "../../include/render/Texture.h"
//#include <FreeImage\FreeImage.h>
#include "../../libs/transferfunction/stb_image.h"
#include <assert.h>
#include <iostream>

Texture::Texture(GLenum TextureTarget, const std::string& filename) : myTextureTarget(TextureTarget)
, myFileName(filename.c_str())
{
	texture_id = Load2DTexture();
}
	
Texture::~Texture()
{
}

void Texture::Bind(GLenum TextureUnit)
{
	glActiveTexture(GL_TEXTURE0 + TextureUnit);
	glBindTexture(myTextureTarget, texture_id);
}

GLuint Texture::GetTextureId()
{
	return texture_id;
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
	GLuint textureID = LoadTexture(myFileName, width, height, &bits, bbp);
	if (textureID == -1)
	{
		assert(false && "Image failed to load 2");
	}

	//glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	//glBindTexture(GL_TEXTURE_2D, textureID);

	// Nice trilinear filtering.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	// Read the file, call glTexImage2D with the right parameters
	//if (bbp == 3)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
	//		0, GL_BGR, GL_UNSIGNED_BYTE, bits);
	//}
	//else if (bbp == 4)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
	//		0, GL_BGRA, GL_UNSIGNED_BYTE, bits);
	//}
	//else
	//{
	//	std::string message = "pixel size = " + std::to_string(bbp) + " don't know how to process this case. I'm out!";
	//	assert(false && message.c_str());
	//}
	
	
	//glGenerateMipmap(GL_TEXTURE_2D);

	//glBindTexture(GL_TEXTURE_2D, 0);

	// Return the ID of the texture we just created
	return textureID;
}

int Texture::LoadTexture(const std::string & fileName, 
	int & width, int & height, unsigned char ** data, int& bbp)
{
	/*
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	BYTE* bits(0);

	GLuint gl_texID;
	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(fileName.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(fileName.c_str());
	}


	//if still unknown, return failure
	if (fif == FIF_UNKNOWN)
	{
		assert(false && "texture unknown file format");
	}


	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, fileName.c_str());

	

	height = FreeImage_GetHeight(dib);
	width = FreeImage_GetWidth(dib);
	bbp = FreeImage_GetBPP(dib);
	bits = FreeImage_GetBits(dib);
	*data = (BYTE *)malloc(width * height * (bbp / 8));
	memcpy(*data, bits, width * height * (bbp / 8));

	//if the image failed to load, return failure
	if (!dib)
	{
		assert(false && "Image failed to load 1");
	}

	//Free FreeImage's copy of the data
	FreeImage_Unload(dib);

	return true;
	*/

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
