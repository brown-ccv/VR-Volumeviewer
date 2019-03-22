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

	std::unique_ptr< Volume <T> > &volume(){
		return m_volume;
	}

	void generateVolume()
	{
		m_volume.reset(new Volume<unsigned short>(m_image_r[0].cols, m_image_r[0].rows, m_image_r.size(), 0.645, 0.645, 5, 3)); //TODO offset variable 
	
		//fill vol and points
		for (int x = 0; x < m_volume->get_width(); x++)
		{
			for (int y = 0; y < m_volume->get_height(); y++)
			{
				for (int z = 0; z < m_volume->get_depth(); z++)
				{
					*(m_volume->get(x, y, z, 0)) = m_image_r[z].at<unsigned short>(y, x);
					*(m_volume->get(x, y, z, 1)) = m_image_g[z].at<unsigned short>(y, x);
				}
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

private:
	unsigned int m_texture_id;
};

#endif // DATA_H
