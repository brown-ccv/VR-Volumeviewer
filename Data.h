//  ----------------------------------
//  Copyright © 2015, Brown University, Providence, RI.
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
///\file Data.h
///\author Benjamin Knorlein
///\date 11/22/2017

#pragma once

#ifndef DATA_H
#define DATA_H

#include <memory>
#include "Volume.h"
#include <vector>
#include <opencv2/core/mat.hpp>

template<class T>
class Data
{
public:
	Data<T>() : m_texture_id(0){};

	~Data()
	{
		
	}

	std::vector<cv::Mat> &image_r()
	{
		return m_image_r;
	}

	std::vector<cv::Mat> &image_g()
	{
		return m_image_g;
	}

	std::vector<cv::Mat> &image_b()
	{
		return m_image_b;
	}

	std::vector<cv::Mat> &image_rgb()
	{
		return m_image_rgb;
	}

	std::unique_ptr< Volume <T> > &volume(){
		return m_volume;
	}
//0.645, 0.645, 5
	void generateVolume(float * resolution)
	{
		if (!m_image_r.empty())
			m_volume.reset(new Volume<unsigned short>(m_image_r[0].cols, m_image_r[0].rows, m_image_r.size(), resolution[0], resolution[1], resolution[2], 3)); //TODO offset variable 
		else if (!m_image_g.empty())
			m_volume.reset(new Volume<unsigned short>(m_image_g[0].cols, m_image_g[0].rows, m_image_g.size(), resolution[0], resolution[1], resolution[2], 3));
		else if (!m_image_b.empty())
			m_volume.reset(new Volume<unsigned short>(m_image_b[0].cols, m_image_b[0].rows, m_image_b.size(), resolution[0], resolution[1], resolution[2], 3));
		else if (!m_image_rgb.empty())
			m_volume.reset(new Volume<unsigned short>(m_image_rgb[0].cols, m_image_rgb[0].rows, m_image_rgb.size() , resolution[0], resolution[1], resolution[2], 3));
		

		//fill vol and points
		for (int z = 0; z < m_volume->get_depth(); z++)
		{
			for (int x = 0; x < m_volume->get_width(); x++)
			{
				//std::cerr << x << " , " << z << " , " << m_image_rgb.size() << std::endl;
				for (int y = 0; y < m_volume->get_height(); y++)
				{			
					if (!m_image_r.empty())
						*(m_volume->get(x, y, z, 0)) = m_image_r[z].at<unsigned short>(y, x);
					if (!m_image_g.empty())
						*(m_volume->get(x, y, z, 1)) = m_image_g[z].at<unsigned short>(y, x);
					if (!m_image_b.empty())
						*(m_volume->get(x, y, z, 2)) = m_image_b[z].at<unsigned short>(y, x);
					if (!m_image_rgb.empty())
					{
						cv::Vec3b rgb = m_image_rgb[z].at<cv::Vec3b>(y, x);		
						*(m_volume->get(x, y, z, 0)) = rgb[0] * 256;
						*(m_volume->get(x, y, z, 1)) = rgb[1] * 256;
						*(m_volume->get(x, y, z, 2)) = rgb[2] * 256;
					}
				}
				//std::cerr << "End " << x << " , " << z << " , " << m_image_rgb.size() << std::endl;

			}
		}	
	}

	unsigned &texture_id() 
	{
		return m_texture_id;
	}

	void set_texture_id(const unsigned texture_id)
	{
		m_texture_id = texture_id;
	}

private:
	
	std::unique_ptr< Volume <T> > m_volume;

	std::vector <cv::Mat> m_image_r;

	std::vector <cv::Mat> m_image_g;

	std::vector <cv::Mat> m_image_b;

	std::vector <cv::Mat> m_image_rgb;

private:
	unsigned int m_texture_id;
};

#endif // DATA_H
