//  ----------------------------------
//  Copyright © 2017, Brown University, Providence, RI.
//
//  All Rights Reserved
//
//  Use of the software is provided under the terms of the GNU General Public License version 3
//  as published by the Free Software Foundation at http://www.gnu.org/licenses/gpl-3.0.html, provided
//  that this copyright notice appear in all copies and that the name of Brown University not be used in
//  advertising or publicity pertaining to the use or distribution of the software without specific written
//  prior permission from Brown University.
//
//  See license.txt for further information.
//
//  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE WHICH IS
//  PROVIDED “AS IS”, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR FOR ANY DAMAGES WHATSOEVER RESULTING
//  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
//  OTHER TORTIOUS ACTION, OR ANY OTHER LEGAL THEORY, ARISING OUT OF OR IN CONNECTION
//  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//  ----------------------------------
//
///\file Volume.cpp
///\author Benjamin Knorlein
///\date 11/6/2017

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/Volume.h"
#include <cstring>
#include <iostream>
#include <math.h>

#include "Texture.h"
#include "stb_image.h"
GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int datatypesize, unsigned int channel, std::string texture_file_path, bool volume_texture_atlas)
	: m_width(width), m_height(height), m_depth(depth), m_channels(channel), m_datatypesize(datatypesize), m_texture_id(0), m_x_scale(x_scale),
	m_y_scale(y_scale), m_z_scale(z_scale), m_render_channel(-1), m_texture_initialized(false), m_pbo_upload_started(false), m_dim(0), m_texture_file_path(texture_file_path), m_volume_texture_atlas(volume_texture_atlas)
{
	if (!volume_texture_atlas)
	{
		// Data is in a 3D array format. Pre-Allocate block of data
		data = new unsigned char[m_width * m_height * m_depth * m_channels * m_datatypesize]();
	}

}

Volume::~Volume()
{
	if (get_texture_id() != 0)
		glDeleteTextures(1, &get_texture_id());
}

void Volume::uploadtoPBO()
{
	glGenBuffers(1, &m_pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, get_depth() * get_width() * get_height() * get_channels() * m_datatypesize, 0, GL_STREAM_DRAW);
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

	memcpy(ptr, data, get_depth() * get_width() * get_height() * get_channels() * m_datatypesize);
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	m_pbo_upload_started = true;
}

void Volume::computeHistogram()
{
	std::vector<std::vector<unsigned int>> m_histogram_tmp;
	for (int i = 0; i < m_channels; i++)
		m_histogram_tmp.push_back(std::vector<unsigned int>(256, 0));

	if (m_datatypesize == 1)
	{
		unsigned char* ptr = reinterpret_cast<unsigned char*>(data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			for (int c = 0; c < get_channels(); c++)
			{
				m_histogram_tmp[c][*ptr]++;
				ptr++;
			}
		}
	}
	else if (m_datatypesize == 2)
	{
		unsigned short* ptr = reinterpret_cast<unsigned short*>(data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			for (int c = 0; c < get_channels(); c++)
			{
				m_histogram_tmp[c][*ptr / 256]++;
				ptr++;
			}
		}
	}
	else if (m_datatypesize == 4)
	{

		float* ptr1 = reinterpret_cast<float*>(data);
		float m_min = std::numeric_limits<float>::max();
		float m_max = std::numeric_limits<float>::min();
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			m_min = std::min(m_min, (*ptr1));
			m_max = std::max(m_max, (*ptr1));
			ptr1++;
		}
		std::cout << "DATA MIN " << m_min << std::endl;
		std::cout << "DATA MAX " << m_max << std::endl;
		std::cout << "COMPUTING HISTOGRAM DATATYPE = 4" << std::endl;
		std::cout << "COMPUTING HISTOGRAM DATATYPE = 4" << std::endl;
		float* ptr = reinterpret_cast<float*>(data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			for (int c = 0; c < get_channels(); c++)
			{
				if ((*ptr) * 255 > 255)
				{
					m_histogram_tmp[c][255]++;
				}
				else
				{
					m_histogram_tmp[c][(*ptr) * 255]++;
				}
				ptr++;
			}
		}
	}

	for (int i = 0; i < m_histogram.size(); i++)
		m_histogram[i].clear();
	m_histogram.clear();

	for (int c = 0; c < m_channels; c++)
	{
		//unsigned int non_black_voxels = get_depth() * get_width() * get_height() - m_histogram_tmp[c][0];
		unsigned int non_black_voxels = get_depth() * get_width() - m_histogram_tmp[c][0];

		m_histogram.push_back(std::vector<float>(m_histogram_tmp[c].size()));
		m_histogram[c][0] = 0;
		for (int i = 1; i < m_histogram_tmp[c].size(); i++)
		{
			m_histogram[c][i] = ((float)m_histogram_tmp[c][i]) / non_black_voxels * 40;
		}
	}
}

void Volume::initGLTextureAtlas()
{
	if (m_texture_initialized)
		return;

	int width;
	int height;
	int bbp;
	stbi_set_flip_vertically_on_load(true);
	stbi_us* t_data = stbi_load_16(m_texture_file_path.c_str(), &width, &height, &bbp, 0);

	
	if (t_data)
	{
		unsigned short* t_prt = t_data;
		unsigned short m_max = std::numeric_limits<unsigned short>::min();
		unsigned short m_min = std::numeric_limits<unsigned short>::max();
		std::vector<std::vector<unsigned int>> m_histogram_tmp;
		m_histogram_tmp.push_back(std::vector<unsigned int>(256, 0));
		for (int i = 0; i < width * height; i++)
		{
			unsigned int index = ((float)*t_prt / (float)(std::numeric_limits<unsigned short>::max() - 1)) * 255;
			m_histogram_tmp[0][index]++;
			m_min = std::min(m_min, (*t_prt));
			m_max = std::max(m_max, (*t_prt));
			t_prt++;
		}

		unsigned int non_black_voxels = get_depth() * get_width() - m_histogram_tmp[0][0];
		for (int i = 0; i < m_histogram.size(); i++)
			m_histogram[i].clear();

		m_histogram.clear();
		m_histogram.push_back(std::vector<float>(m_histogram_tmp[0].size()));
		m_histogram[0][0] = 0;
		//std::cout << 0 <<": "<< m_histogram_tmp[0][0] <<std::endl;
		for (int i = 1; i < m_histogram_tmp[0].size(); i++)
		{
			//std::cout << i <<": "<< m_histogram_tmp[0][i] <<std::endl;
			m_histogram[0][i] = (((float)m_histogram_tmp[0][i]) / non_black_voxels) * 32000;
		}

		std::cout << "VOLUME DATA MIN " << m_min << std::endl;
		std::cout << "VOLUME DATA MAX " << m_max << std::endl;

		std::cout << "volume bbp: " << bbp << std::endl;

		std::cout << "LOADING VOLUME TEXTURE" << std::endl;
		glCheckError();
		if (get_texture_id() != 0)
		{
			glDeleteTextures(1, &m_texture_2_id);
		}
		glCheckError();


		glGenTextures(1, &m_texture_2_id);
		glCheckError();
		glBindTexture(GL_TEXTURE_2D, m_texture_2_id);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			GL_REPEAT);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			GL_REPEAT);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glCheckError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
		glCheckError();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, t_data);
		glCheckError();

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(t_data);
		std::cout << "END LOADING VOLUME TEXTURE" << std::endl;
		m_dim = ceil(sqrt(m_depth));
		set_volume_scale({ static_cast<float>(1.0f / (m_x_scale * (m_width / m_dim) )),
						   static_cast<float>(1.0f / (m_y_scale * (m_height / m_dim) )),
						   static_cast<float>(1.0f / (m_z_scale * m_depth)) });
		
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}


}

void Volume::initGL()
{
	
	if (m_texture_initialized)
		return;

	if (m_volume_texture_atlas)
	{
		initGLTextureAtlas();
		m_texture_initialized = true;
		return;
	}

	if (!m_pbo_upload_started)
	{
		uploadtoPBO();
		return;
	}

	if (get_texture_id() != 0)
		glDeleteTextures(1, &get_texture_id());

	glGenTextures(1, &get_texture_id());
	glBindTexture(GL_TEXTURE_3D, get_texture_id());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,
		GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,
		GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,
		GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);

	switch (m_channels)
	{
	case 1:
	{
		if (m_datatypesize == 1)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
		}
		if (m_datatypesize == 4)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, get_width(), get_height(), get_depth(), 0, GL_RED, GL_FLOAT, NULL);
		}
	}
	break;
	case 3:
	{
		if (m_datatypesize == 1)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
		}
	}

	break;
	case 4:
	{
		if (m_datatypesize == 1)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_SHORT, NULL);
		}
	}
	break;

	default:
	{
		{
			if (m_datatypesize == 1)
			{
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}
			if (m_datatypesize == 2)
			{
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
			}
		}
		break;
	}
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &m_pbo);
	m_pbo_upload_started = false;

	glGenerateMipmap(GL_TEXTURE_3D);
	set_volume_scale({ static_cast<float>(1.0f / (m_x_scale * m_width)),
					  static_cast<float>(1.0f / (m_y_scale * m_height)),
					  static_cast<float>(1.0f / (m_z_scale * m_depth)) });

	delete[] data;
	data = nullptr;

	m_texture_initialized = true;
}