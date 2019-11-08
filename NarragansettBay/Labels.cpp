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
///\file Labels.cpp
///\author Benjamin Knorlein
///\date 6/25/2019

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#define M_PI 3.14159265358979323846
#define MARKER_HEIGHT 0.05

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

#include "Labels.h"
#include "Glider.h"
#include <string>
#include <math/VRMath.h>
#include "VRFontHandler.h"

Labels::Labels(Glider * glider) : m_glider(glider)
{
}

Labels::~Labels()
{
}

void Labels::parse()
{
	for (int i = 1; i < m_glider->date(2).size() - 1; i++)
		{
			if (m_glider->date(2)[i - 1] != m_glider->date(2)[i])
			{
				marker.push_back(std::to_string(m_glider->date(2)[i]) + ":00");
				marker_idx.push_back(i);
			}
		}
		for (int i = 1; i < marker_idx.size() - 1; i++)
		{
			if (m_glider->date(1)[marker_idx[i] - 1] != m_glider->date(1)[marker_idx[i]])
			{
				marker[i] = std::to_string(m_glider->date(0)[0]) + "/" + std::to_string(m_glider->date(1)[1]) + " - " + marker[i];
			}
		}
}

void Labels::draw(glm::mat4 &MV, glm::mat4 &headpose)
{
	
	for (int i = 0; i < marker.size(); i++)
	{ 
		/*glm::vec4 markerpos = MV *  glm::vec4(m_glider->positions()[marker_idx[i]].vertex[0], m_glider->positions()[marker_idx[i]].vertex[1], 1, 1);
		glm::vec4 headpos = headpose *  glm::vec4(0, 0, 0,1);
		glm::vec4 dir = headpos / headpos.w - markerpos / markerpos.w;
		dir.w = 0;
		dir = inverse(MV)  * dir;
		dir.z = 0;
		dir = normalize(dir);
		double angle = 180.0f / M_PI * atan2(dir.y, dir.x);
		*/
		glBegin(GL_LINES);
		glColor3f(0.5f, 0.5f, 0.0f);     // Yellow	
		glVertex3f(m_glider->positions()[marker_idx[i]].vertex[0], m_glider->positions()[marker_idx[i]].vertex[1], m_glider->positions()[marker_idx[i]].vertex[2]);
		glVertex3f(m_glider->positions()[marker_idx[i]].vertex[0], m_glider->positions()[marker_idx[i]].vertex[1], 1.1);
		glEnd();  // End of drawing color-cube	

		glPushMatrix();
		glTranslatef(m_glider->positions()[marker_idx[i]].vertex[0], m_glider->positions()[marker_idx[i]].vertex[1], 1.1);
		glRotatef(90, 1, 0, 0);
		//glRotatef(angle, 0, 0, 1);

		glColor3f(0.5f, 0.5f, 0.5f);
		VRFontHandler::getInstance()->renderTextBox(marker[i], -1000,
			0.0, 0, 2000.0, 0.05);
		

		
		glPopMatrix();
	}
}
