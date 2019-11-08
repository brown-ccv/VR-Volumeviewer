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
///\file Glider.cpp
///\author Benjamin Knorlein
///\date 6/25/2019

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#define THICKNESS 0.001f

#if defined(WIN32)
	#define NOMINMAX
	#include <windows.h>
	#include <GL/gl.h>
	#include <gl/GLU.h>
#elif defined(__APPLE__)
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/glu.h>
#else
	#define GL_GLEXT_PROTOTYPES
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <GL/gle.h>

#include "Glider.h"
#include <iostream>
#include <glm/gtc/type_ptr.inl>

Glider::Glider() : m_current_value(0), m_display_list(0)
{
	m_tool = new Tool(this);
	m_label = new Labels(this);
}

Glider::~Glider()
{
	if (m_display_list > 0)
		glDeleteLists(m_display_list,1);

	delete m_tool;
	delete m_label;
}

void Glider::updateList(){

	if (m_display_list > 0)
		glDeleteLists(m_display_list, 1);

	m_display_list = glGenLists(1);
	glNewList(m_display_list, GL_COMPILE);
	double pts[12000][3];
	float col[12000][4];
	if (m_current_value < values().size()){
		for (int i = 0; i < values()[m_current_value].size(); i++)
		{
			if (values()[m_current_value][i] == -9999999)
			{
				col[i][0] = 0.5f; col[i][1] = 0.5f; col[i][2] = 0.5f; col[i][3] = 0.0f;
			}
			else{
//				TransferFunction::getJetColor(values()[m_current_value][i], m_min_max[m_current_value].first, m_min_max[m_current_value].second, col[i][0], col[i][1], col[i][2], col[i][3]);
			}
			col[i][3] = 1.0f;
			pts[i][0] = m_positions[i].vertex[0];
			pts[i][1] = m_positions[i].vertex[1];
			pts[i][2] = m_positions[i].vertex[2];
		}
	}

	gleSetJoinStyle(TUBE_JN_RAW);
	gleSetNumSides(6);
	glePolyCylinder_c4f(m_positions.size(), pts, col, THICKNESS);
	
	glEndList();
}

void Glider::draw()
{
	if (m_display_list <= 0){
		updateList();
	}
	
	glCallList(m_display_list);
}

void Glider::drawTool(glm::mat4 controllerpose)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(controllerpose));
	m_tool->draw(m_current_value);
	glPopMatrix();
}

void Glider::drawLabels(glm::mat4& MV, glm::mat4& headpose)
{
	m_label->draw(MV, headpose);
}
