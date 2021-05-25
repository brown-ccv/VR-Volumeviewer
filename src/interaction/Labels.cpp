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

#include "../../include/interaction/Labels.h"
#include <string>
#include <math/VRMath.h>
#include "../../include/render/FontHandler.h"
#include <glm/gtc/type_ptr.inl>

Labels::Labels()
{

}

Labels::~Labels()
{
  clear();
}

void Labels::clear() {
  m_text.clear();
  m_position.clear();
  m_position2.clear();
  m_size.clear();
  m_volume.clear();
}

void Labels::add(std::string text, float x, float y, float z, float textPosZ, float size, int volume)
{
  m_text.push_back(text);
  m_position.push_back(glm::vec3(x, y, z));
  m_position2.push_back(glm::vec3(x, y, textPosZ));
  m_size.push_back(size);
  m_volume.push_back(volume);
}

void Labels::draw(std::vector<glm::mat4>& MV, glm::mat4& headpose, float z_scale)
{
  for (int i = 0; i < m_text.size(); i++)
  {
    glm::vec4 markerpos = MV[m_volume[i]] * glm::vec4(m_position[i].x, m_position[i].y, 1, 1);
    glm::vec4 headpos = headpose * glm::vec4(0, 0, 0, 1);
    glm::vec4 dir = headpos / headpos.w - markerpos / markerpos.w;
    dir.w = 0;
    dir = inverse(MV[m_volume[i]]) * dir;
    dir.z = 0;
    dir = normalize(dir);
    double angle = 180.0f / M_PI * atan2(dir.x, dir.y);

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(MV[m_volume[i]]));
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    // Yellow	
    glVertex3f(m_position[i].x, m_position[i].y, m_position[i].z);
    glVertex3f(m_position[i].x, m_position[i].y, m_position2[i].z - 3);
    glEnd();

    glPushMatrix();
    glTranslatef(m_position[i].x, m_position[i].y, m_position2[i].z);
    glRotatef(180 - angle, 0, 0, 1);
    glRotatef(90, 1, 0, 0);
    glScalef(1.0f, 1.0f / z_scale, 1.0f);
    FontHandler::getInstance()->renderTextBox(m_text[i], -1000.0, 0, 0, 2000.0, m_size[i], TextAlignment::CENTER);
    glEnable(GL_LIGHTING);




    glPopMatrix();
    glPopMatrix();
  }
}
