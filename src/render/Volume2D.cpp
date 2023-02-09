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

#include "render/Volume2D.h"
#include <cstring>
#include <iostream>
#include <math.h>
#include "stb_image.h"

Volume2D::Volume2D(unsigned int width, unsigned int height,
	unsigned int depth, double x_scale, double y_scale,
	double z_scale, unsigned int datatypesize, unsigned int channel, std::string texture_file_path)
	: Volume(width,height ,depth , x_scale, y_scale, z_scale,  datatypesize, channel),m_texture_file_path(texture_file_path)
{
	m_is_texture_atlas = true;
}


Volume2D::Volume2D(const Volume2D& other):Volume(other)
{

}

Volume2D::~Volume2D()
{

}

void Volume2D::computeHistogram()
{
	unsigned short* t_prt = reinterpret_cast<unsigned short*>(m_data);
	unsigned short m_max = std::numeric_limits<unsigned short>::min();
	unsigned short m_min = std::numeric_limits<unsigned short>::max();
	std::vector<std::vector<unsigned int>> m_histogram_tmp;
	m_histogram_tmp.push_back(std::vector<unsigned int>(256, 0));
	for (int i = 0; i < m_width * m_height; i++)
	{
		unsigned int index = ((float)*t_prt / (float)(std::numeric_limits<unsigned short>::max() - 1)) * 255;
		m_histogram_tmp[0][index]++;
		m_min = std::min(m_min, (*t_prt));
		m_max = std::max(m_max, (*t_prt));
		t_prt++;
	}

	std::cout << "VOLUME DATA MIN " << m_min << std::endl;
	std::cout << "VOLUME DATA MAX " << m_max << std::endl;

	unsigned int non_black_voxels = m_height * m_width - m_histogram_tmp[0][0];
	for (int i = 0; i < m_histogram.size(); i++)
		m_histogram[i].clear();

	m_histogram.clear();
	m_histogram.push_back(std::vector<float>(m_histogram_tmp[0].size()));
	m_histogram[0][0] = 0;

	for (int i = 1; i < m_histogram_tmp[0].size(); i++)
	{
		m_histogram[0][i] = ((float)m_histogram_tmp[0][i]) / non_black_voxels * 40;
	}

}

void Volume2D::initGL()
{

	if (m_texture_initialized)
		return;


	int bbp;
	stbi_set_flip_vertically_on_load(true);
	m_data = stbi_load_16(m_texture_file_path.c_str(), &m_width, &m_height, &bbp, 0);
	std::cout << "channels in file : " << bbp << std::endl;
	computeHistogram();

	if (m_data)
	{
		
		
		if (get_texture_id() != 0)
		{
			glDeleteTextures(1, &m_texture_id);
		}
		

		glGenTextures(1, &m_texture_id);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, m_width, m_height, 0, GL_RED, GL_UNSIGNED_SHORT, m_data);
		

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(m_data);
		std::cout << "END LOADING VOLUME TEXTURE" << std::endl;
		m_dim = ceil(sqrt(m_depth));
		set_volume_scale({ static_cast<float>(1.0f / (m_x_scale * (m_width / m_dim))),
						   static_cast<float>(1.0f / (m_y_scale * (m_height / m_dim))),
						   static_cast<float>(1.0f / (m_z_scale * m_depth)) });

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	m_texture_initialized = true;
}