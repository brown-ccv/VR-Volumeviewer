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
///\file Tool.cpp
///\author Benjamin Knorlein
///\date 6/25/2019

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

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

#include "Tool.h"
#include "Glider.h"
#include "VRFontHandler.h"

Tool::Tool(Glider * glider) : m_glider(glider)
{
}

Tool::~Tool()
{
}

void Tool::draw(int listId)
{
		int step_length = 101;
		double width = 0.025;
		double length = 0.5 / step_length;
		double offset = 0.15;

		//draw Legend
		//if (measuring){
			double step = (m_glider->min_max()[listId].second - m_glider->min_max()[listId].first) / step_length;
			glBegin(GL_QUAD_STRIP);
			float r, g, b, a;
			for (int i = 0; i < step_length; i++)
			{
				//TransferFunction::getJetColor(m_glider->min_max()[listId].first + step* i, m_glider->min_max()[listId].first, m_glider->min_max()[listId].second, r, g, b, a);
				glColor3f(r, g, b);
				glVertex3f(-width, 0, -length*i - offset);
				glVertex3f(width, 0, -length*i - offset);
			}
			glEnd();
	
			glPushMatrix();
			glRotatef(-90, 1, 0, 0);
			glColor3f(0.5f, 0.5f, 0.5f);
			for (int i = 0; i < step_length; i++)
			{
	
				if (i % 10 == 0)
				{
					glBegin(GL_LINES);
					glVertex3f(width, length*i + offset, 0);
					glVertex3f(width + 0.025, length*i + offset, 0);
					glEnd();
	
					char buf[20];
					sprintf(buf, "%.3lf", m_glider->min_max()[listId].first + i * step);
					std::string text = std::string(buf);
					VRFontHandler::getInstance()->renderTextBox(text,
						width + 0.025,
						length*i + offset - 0.015,
						0,
						0.5, 0.03, VRFontHandler::LEFT);
				}
			}
			glPopMatrix();
		//}
	
		//Add text
		glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		glColor3f(0.5f, 0.5f, 0.5f);
		VRFontHandler::getInstance()->renderTextBox(m_glider->values_legend()[listId],
			-0.5,
			0.05,
			0.0,
			1.0, 0.03, VRFontHandler::CENTER);
	
		glPopMatrix();

		//Add currently measured value

		//if (closestPoint >= 0 && points[currentScale][currentList][closestPoint].value != -9999999){
		//	glColor3f(0.7f, 0.7f, 0.7f);     // Yellow

		//	double width = 0.025;
		//	double offset = 0.15;
		//	double pos = 0.5 * (min_max[1][currentList] - points[currentScale][currentList][closestPoint].value) / (min_max[1][currentList] - min_max[0][currentList]);

		//	VRFontHandler::getInstance()->renderTextBox(std::to_string(points[currentScale][currentList][closestPoint].value),
		//		-0.5,
		//		0.0,
		//		-(0.5 - pos + offset),
		//		1.0, 0.1
		//		);


		//	glPushMatrix();
		//	glRotatef(-90, 1, 0, 0);
		//	glColor3f(0.5f, 0.5f, 0.5f);
		//	char date_str[50];
		//	sprintf(date_str, "%d/%d - %02d:%02d:%02d", date[0][closestPoint], date[1][closestPoint], date[2][closestPoint], date[3][closestPoint], date[4][closestPoint]);

		//	VRFontHandler::getInstance()->renderTextBox(std::string(date_str),
		//		-0.5,
		//		0.1,
		//		0.0,
		//		1.0, 0.03, VRFontHandler::CENTER);

		//	glPopMatrix();
		//}
}
