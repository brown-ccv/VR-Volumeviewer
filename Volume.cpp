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
#include <iostream>

Volume::Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int datatypesize, unsigned int channel)
	: m_width{ width }, m_height{ height }, m_depth{ depth }, m_channels{ channel }, m_datatypesize{ datatypesize }, m_texture_id{ 0 }
	, m_x_scale{ x_scale }, m_y_scale{ y_scale }, m_z_scale{ z_scale }, m_render_channel(-1), m_texture_initialized(false), m_pbo_upload_started{false}
{
	data = new unsigned char[m_width*m_height*m_depth * m_channels * m_datatypesize]();
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
	glBufferData(GL_PIXEL_UNPACK_BUFFER, get_depth()*get_width()*get_height() * get_channels() * m_datatypesize, 0, GL_STREAM_DRAW);
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	//updatePixels(ptr, get_depth() * get_width() * get_height());
	memcpy(ptr, get_data(), get_depth() * get_width() * get_height() * get_channels() * m_datatypesize);
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	m_pbo_upload_started = true;
}


void Volume::computeHistogram()
{
	std::vector< std::vector<unsigned int> > m_histogram_tmp;
	for (int i = 0; i < m_channels; i++)
		m_histogram_tmp.push_back(std::vector<unsigned int>(256, 0));

	if (m_datatypesize == 1) {
		unsigned char* ptr = reinterpret_cast <unsigned char*> (data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++ )
		{
			for(int c = 0 ; c < get_channels();c++)
			{
				m_histogram_tmp[c][*ptr]++;
				ptr++;
			}
		}
	}
	else if (m_datatypesize == 2) {
		unsigned short* ptr = reinterpret_cast <unsigned short*> (data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			for (int c = 0; c < get_channels(); c++)
			{
				m_histogram_tmp[c][*ptr/256]++;
				ptr++;
			}
		}
	}
	else if (m_datatypesize == 4) {
		float* ptr = reinterpret_cast <float*> (data);
		for (int i = 0; i < get_depth() * get_width() * get_height(); i++)
		{
			for (int c = 0; c < get_channels(); c++)
			{
				m_histogram_tmp[c][*ptr*255]++;
				ptr++;
			}
		}
	}

	for (int i = 0; i < m_histogram.size(); i++)
		m_histogram[i].clear();
	m_histogram.clear();
	
	for (int c = 0; c < m_channels; c++) {
		unsigned int nbVoxel = get_depth() * get_width() * get_height() - m_histogram_tmp[c][0];

		m_histogram.push_back(std::vector<float>(m_histogram_tmp[c].size()));
		m_histogram[c][0] = 0; 
		for (int i = 1; i < m_histogram_tmp[c].size(); i ++)
		{
			m_histogram[c][i] = ((float) m_histogram_tmp[c][i]) / nbVoxel * 30;
		}
	}
}

void Volume::initGL()
{
	if (m_texture_initialized)
		return;

	if(!m_pbo_upload_started)
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

	if (m_channels == 3){
		if (m_datatypesize == 1){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, get_width(), get_height(), get_depth(), 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
		}
	}
	else if (m_channels == 4)
	{
		if (m_datatypesize == 1){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, get_width(), get_height(), get_depth(), 0, GL_RGBA, GL_UNSIGNED_SHORT, NULL);
		}
	}
	else if (m_channels == 1)
	{
		if (m_datatypesize == 1){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		}
		if (m_datatypesize == 2){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, get_width(), get_height(), get_depth(), 0, GL_RED, GL_UNSIGNED_SHORT, NULL);
		}
		if (m_datatypesize == 4){
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, get_width(), get_height(), get_depth(), 0, GL_RED, GL_FLOAT, NULL);
		}
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glDeleteBuffers(1, &m_pbo);
	m_pbo_upload_started = false;
	
	glGenerateMipmap(GL_TEXTURE_3D);
	set_volume_scale({
		static_cast<float>(1.0f / (get_x_scale() * get_width())),
		static_cast<float>(1.0f / (get_y_scale() * get_height())),
		static_cast<float>(1.0f / (get_z_scale() * get_depth()))
	});

	delete[] data;
	data = nullptr;

	m_texture_initialized = true;
	
}
