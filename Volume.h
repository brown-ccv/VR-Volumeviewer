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

#include <cmath>

	template <class T>
	class Volume
	{
	public:
		Volume(unsigned int width, unsigned int height, unsigned int depth, double x_scale, double y_scale, double z_scale, unsigned int channel = 1)
			: m_width{ width }, m_height{ height }, m_depth{ depth }, m_channels{ channel }
		, m_x_scale{ x_scale }, m_y_scale{ y_scale }, m_z_scale{ z_scale }
		{
			data = new T[m_width*m_height*m_depth * m_channels]();
		}

		~Volume()
		{
			delete[] data;
		}

		T *get(int x, int y, int z, int channel = 0)
		{
			return &data[(x + (y + z * m_height)* m_width) * m_channels + channel];
		}

		double getNonInterpolated(int x, int y, int z, int channel = 0)
		{
			double x_s = x / m_x_scale;
			double y_s = y / m_y_scale;
			double z_s = z / m_z_scale;

			if (x_s < 0 || y_s < 0 || z_s < 0
				|| x_s >(m_width - 1) || y_s > m_height - 1 || z_s > m_depth - 1)
				return 0;

			return *get(x_s, y_s, z_s, channel);

		}

		double getInterpolated(double x, double y, double z, int channel = 0)
		{
			double x_s = x / m_x_scale;
			double y_s = y / m_y_scale;
			double z_s = z / m_z_scale;

			if (x_s < 0 || y_s < 0 || z_s < 0
				|| x_s >(m_width - 1) || y_s > m_height - 1 || z_s > m_depth - 1)
				return 0;

			unsigned int x_0 = floor(x_s);
			unsigned int y_0 = floor(y_s);
			unsigned int z_0 = floor(z_s);

			double x_d = (x_s - x_0);
			double y_d = (y_s - y_0);
			double z_d = (z_s - z_0);

			double c_00 = *get(x_0, y_0, z_0, channel) * (1 - x_d) + *get(x_0 + 1, y_0, z_0, channel) * x_d;
			double c_01 = *get(x_0, y_0, z_0 + 1, channel) * (1 - x_d) + *get(x_0 + 1, y_0, z_0 + 1, channel) * x_d;
			double c_10 = *get(x_0, y_0 + 1, z_0, channel) * (1 - x_d) + *get(x_0 + 1, y_0 + 1, z_0, channel) * x_d;
			double c_11 = *get(x_0, y_0 + 1, z_0 + 1, channel) * (1 - x_d) + *get(x_0 + 1, y_0 + 1, z_0 + 1, channel) * x_d;

			double c_0 = c_00 * (1 - y_d) + c_10 * y_d;
			double c_1 = c_01 * (1 - y_d) + c_11 * y_d;

			return c_0 * (1 - z_d) + c_1 * z_d;
		}

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

	private:
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_depth;

		unsigned int m_channels;

		double m_x_scale;
		double m_y_scale;
		double m_z_scale;

		T * data;
	};

#endif // VOLUME_H
