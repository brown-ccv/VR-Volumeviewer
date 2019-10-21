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
///\file ArcBall.cpp
///\author Benjamin Knorlein
///\date 10/17/2019


#include "ArcBall.h"
#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <iostream>

#ifndef _PI
#define _PI 3.141592653
#endif

ArcBall::ArcBall() : m_radius(1) ,m_mouse_left_pressed(false),m_mouse_right_pressed(false),last_x(0), last_y(0)
							, m_PanFactor(0.1), m_RotateFactor(0.1), m_cameraScrollFactor(0.1), m_target(0,0,0), m_eye(0, 0, 1),  m_up(0, 1, 0)
	{
		

	}

	ArcBall::~ArcBall()
	{
	
	}

	void ArcBall::updateCameraMatrix()
	{
		m_eye = m_radius * glm::normalize(m_eye);
		viewmatrix = glm::lookAt(m_target + m_eye, m_target, m_up);
	}

	void ArcBall::mouse_pressed(int button, bool isDown)
	{
		if(button == 0) //left -> rotate
		{
			m_mouse_left_pressed = isDown;
		}
		else if(button == 1) // right ->pan
		{
			m_mouse_right_pressed = isDown;
			
		}
		else if(button == 2)// mouse_wheel -> zoom
		{
			if(isDown)
				Zoom((float)m_cameraScrollFactor);
			else
				Zoom((float)-m_cameraScrollFactor);
		}
	}

	void ArcBall::mouseMove(float x, float y){
		if (m_mouse_left_pressed) {
			// Calculate the new phi and theta based on mouse position relative to where the user clicked
			float dx = ((float)(last_x - x)) / 300.0f;
			float dy = ((float)(last_y - y)) / 300.0f;

			Rotate(dx * m_RotateFactor, -dy * m_RotateFactor);
		}
		else if (m_mouse_right_pressed) {
			float dx = ((float)(last_x - x)) / 300.0f;
			float dy = ((float)(last_y - y)) / 300.0f;

			Pan(-dx * m_PanFactor, -dy * m_PanFactor);
		}
	
		last_x = x;
		last_y = y;
	}

void ArcBall::Rotate(float dx, float dy) {

	glm::vec3  right = glm::cross(m_eye, m_up);
	glm::mat3 rot = glm::rotate(glm::degrees(dy), right) * glm::rotate(glm::degrees(dx), m_up);
	
	m_eye = rot * m_eye;
	m_up = rot * m_up;
}

void ArcBall::Zoom(float distance) {
	m_radius -= distance;
}

void ArcBall::Pan(float dx, float dy) {

	glm::vec3  right = glm::cross(m_eye, m_up);
	
	m_target = m_target + (right * dx) + (m_up * dy);
}



