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

//GLenum glCheckError_(const char* file, int line)
//{
//	GLenum errorCode;
//	while ((errorCode = glGetError()) != GL_NO_ERROR)
//	{
//		std::string error;
//		switch (errorCode)
//		{
//		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
//		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
//		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
//		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
//		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
//		}
//		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
//	}
//	return errorCode;
//}
//#define glCheckError() glCheckError_(__FILE__, __LINE__)

Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int datatypesize, unsigned int channel)
	: m_width(width), m_height(height), m_depth(depth), m_channels(channel), m_datatypesize(datatypesize), m_texture_id(0), m_x_scale(x_scale),
	m_y_scale(y_scale), m_z_scale(z_scale), m_render_channel(-1), m_texture_initialized(false), m_pbo_upload_started(false), m_dim(0), m_is_texture_atlas(false)
{

}

Volume::Volume(const Volume& other) :m_width(other.m_width), m_height(other.m_height),
									m_depth(other.m_depth), m_channels(other.m_channels),
									m_datatypesize(other.m_datatypesize), m_texture_id(other.m_texture_id),
									m_x_scale(other.m_x_scale),m_y_scale(other.m_y_scale), m_z_scale(other.m_z_scale),
									m_render_channel(other.m_render_channel), m_texture_initialized(other.m_texture_initialized), 
									m_pbo_upload_started(other.m_pbo_upload_started), m_dim(other.m_dim), m_is_texture_atlas(other.m_is_texture_atlas)
{

}

Volume::~Volume()
{
	// In the 2D case, the data array was release by stbi library.
	// In the 3D case, the allocation is made manually.
	if (!m_is_texture_atlas)
	{
		delete m_data;
	}
	
	if (m_texture_id != 0)
		glDeleteTextures(1, &m_texture_id);
}
