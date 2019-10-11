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

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Volume.h"

Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int datatypesize, unsigned int channel)
	: m_width{ width }, m_height{ height }, m_depth{ depth }, m_channels{ channel }, m_datatypesize{ datatypesize }, m_texture_id{ 0 }
	, m_x_scale{ x_scale }, m_y_scale{ y_scale }, m_z_scale{ z_scale }, m_render_channel(-1), m_texture_initialized(false)
{
	data = new unsigned char[m_width*m_height*m_depth * m_channels * m_datatypesize]();
}

Volume::~Volume()
{
	if (get_texture_id() != 0)
		glDeleteTextures(1, &get_texture_id());
}

void Volume::initGL()
{
	if (m_texture_initialized)
		return;
	
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

	if (m_channels == 3){
		if (m_datatypesize == 1){
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_BYTE, get_data());
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_SHORT, get_data());
		}
	}
	else if (m_channels == 4)
	{
		if (m_datatypesize == 1){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, get_data());
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_SHORT, get_data());
		}
	}
	else if (m_channels == 1)
	{
		if (m_datatypesize == 1){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_BYTE, get_data());
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_SHORT, get_data());
		}
		if (m_datatypesize == 4){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, get_width(), get_height(), get_depth(), 0, GL_RED, GL_FLOAT, get_data());
		}
	}


	glGenerateMipmap(GL_TEXTURE_3D);
	set_volume_scale({
		static_cast<float>(1.0f / (get_x_scale() * get_width())),
		static_cast<float>(1.0f / (get_y_scale() * get_height())),
		static_cast<float>(1.0f / (get_z_scale() * get_depth()))
	});

	delete[] data;

	m_texture_initialized = true;
	
}
