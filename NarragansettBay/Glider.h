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
///\file Glider.h
///\author Benjamin Knorlein
///\date 6/25/2019

#pragma once

#ifndef GLIDER_H
#define GLIDER_H

#include <vector>
#include "Tool.h"
#include <glm/mat4x4.hpp>
#include "Labels.h"

struct _pt {
	float vertex[3];
	_pt(float x, float y, float z)
	{
		vertex[0] = x;
		vertex[1] = y;
		vertex[2] = z;
	};
};

class Glider
{
public:
	Glider();
	~Glider();
	
	void updateList();

	void draw();

	std::vector<int>& date(int i)
	{
		return m_date[i];
	}

	std::vector < _pt >& positions()
	{
		return m_positions;
	}

	std::vector<std::string>& values_legend()
	{
		return m_values_legend;
	}

	std::vector< std::vector<float> >& values()
	{
		return m_values;
	}

	std::vector<std::pair < float, float>>& min_max()
	{
		return m_min_max;
	}

	void drawTool(glm::mat4 controllerpose);
	void drawLabels(glm::mat4 &MV, glm::mat4 &headpose);

	void parseLabels(){ m_label->parse(); }

private:
	//date in //month - day - hour - minute - second
	std::vector<int> m_date[5];
	std::vector < _pt >	m_positions;

	std::vector<std::string> m_values_legend;
	std::vector< std::vector<float> >m_values;
	std::vector <std::pair < float,float> > m_min_max;

	int m_current_value;
	unsigned int m_display_list;

	Tool * m_tool;
	Labels * m_label;
};

#endif // GLIDER_H
