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
///\file Volume.h
///\author Benjamin Knorlein
///\date 11/6/2017

#pragma once

#ifndef VOLUME_H
#define VOLUME_H

#include "GL/glew.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "TransferFunction.h"
struct pt
{
	float x;
	float y;
	float z;
};

enum RenderType
{
	SLICE_RENDERER,
	RAYCAST_RENDERER
};

	class Volume
	{
	public:
		Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int datatypesize, unsigned int channel = 1);

		~Volume();
		

		unsigned get_width() const
		{
			return m_width;
		}

		unsigned get_height() const
		{
			return m_height;
		}

		unsigned get_depth() const
		{
			return m_depth;
		}

		unsigned get_channels() const
		{
			return m_channels;
		}

		double get_x_scale() const
		{
			return m_x_scale;
		}

		double get_y_scale() const
		{
			return m_y_scale;
		}

		double get_z_scale() const
		{
			return m_z_scale;
		}

		unsigned& get_texture_id() 
		{
			return m_texture_id;
		}

		void set_texture_id(const unsigned texture_id)
		{
			m_texture_id = texture_id;
		}

		const pt& get_volume_position() const
		{
			return m_volume_position;
		}

		void set_volume_position(const pt pt)
		{
			m_volume_position = pt;
		}

		const pt& get_volume_scale() const
		{
			return m_volume_scale;
		}

		void set_volume_scale(const pt pt)
		{
			m_volume_scale = pt;
		}

		const glm::mat4& get_volume_mv() const
		{
			return m_volume_MV;
		}

		void set_volume_mv(const glm::mat4 highp_mat4_x4)
		{
			m_volume_MV = highp_mat4_x4;
		}

		unsigned char* get_data()
		{
			return data;
		}

		const RenderType& render_type() const
		{
			return m_renderType;
		}

		void set_render_type(const RenderType render)
		{
			m_renderType = render;
		}

		const int& render_channel() const
		{
			return m_render_channel;
		}

		void set_render_channel(const int render_channel)
		{
			m_render_channel = render_channel;
		}

		void set_transfer_function(TransferFunction * transfer_function)
		{
			m_transfer_function = transfer_function;
		}

		TransferFunction * transfer_function() const
		{
			return m_transfer_function;
		}

		void initGL();
		

	private:
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_depth;

		unsigned int m_channels;

		double m_x_scale;
		double m_y_scale;
		double m_z_scale;

		unsigned int m_texture_id;

		pt m_volume_position;
		pt m_volume_scale;
		glm::mat4 m_volume_MV;

		
		unsigned int m_datatypesize;
		unsigned char * data;

		RenderType m_renderType;
		int m_render_channel;

		TransferFunction *m_transfer_function;
	};

#endif // VOLUME_H
