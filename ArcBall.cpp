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
#include <glm/gtx/string_cast.hpp>

#ifndef _PI
#define _PI 3.141592653
#endif

ArcBall::ArcBall() : m_radius(1) ,m_mouse_left_pressed(false), m_mouse_center_pressed(false), m_mouse_right_pressed(false),last_x(0), last_y(0)
, m_PanFactor(1), m_RotateFactor(1), m_cameraScrollFactor(0.1), m_target(0, 0, 0), m_eye(0, 0, 1), m_up(0, 1, 0), m_rotate_camera_center{false}
	{
		

	}

	ArcBall::~ArcBall()
	{
	
	}

	void ArcBall::updateCameraMatrix()
	{
		m_eye = glm::normalize(m_eye);
		viewmatrix = glm::lookAt(m_radius * m_eye + m_target, m_target, m_up);
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
		else if(button == 2)
		{
			
			m_mouse_center_pressed = isDown;
		}
	}

	void ArcBall::mouse_move(float x, float y){
		if (m_mouse_left_pressed) {
			// Calculate the new phi and theta based on mouse position relative to where the user clicked
			float dx = ((float)(last_x - x)) / 300.0f;
			float dy = ((float)(last_y - y)) / 300.0f;

			Rotate(dx * m_RotateFactor, -dy * m_RotateFactor);
		}
		else if (m_mouse_center_pressed) {
			float dy = ((float)(last_y - y)) / 300.0f;

			RotateEyeAxis( dy * m_RotateFactor);
		}
		else if (m_mouse_right_pressed) {
			float dx = ((float)(last_x - x)) / 300.0f;
			float dy = ((float)(last_y - y)) / 300.0f;

			Pan(-dx * m_PanFactor, -dy * m_PanFactor);
		}
	
		last_x = x;
		last_y = y;
	}

void ArcBall::mouse_scroll(float dist) {
	Zoom(dist);
}

void ArcBall::setCameraCenterRotation(bool useCameraCenter) {
	if (useCameraCenter != m_rotate_camera_center) {
		m_rotate_camera_center = useCameraCenter;
		if (!m_rotate_camera_center) {
			m_target = glm::vec3{ 0.0 };
		}
	}
}

void ArcBall::wasd_pressed(int awsd) {
	glm::vec3 dir = glm::normalize(-m_eye);
	glm::vec3  right = glm::cross(dir, m_up);

	if (W & awsd) {
		m_target = m_target + dir * glm::vec3(0.001);
		updateCameraMatrix();
	}
	if (S & awsd) {
		m_target = m_target - dir * glm::vec3(0.001);
		updateCameraMatrix();
	}
	if (A & awsd) {
		m_target = m_target - (right) * glm::vec3(0.001);
		updateCameraMatrix();
	}
	if (D & awsd) {
		m_target = m_target + (right) * glm::vec3(0.001);
		updateCameraMatrix();
	}
	if (Q & awsd) {
		m_target = m_target - (m_up)* glm::vec3(0.001);
		updateCameraMatrix();
	}
	if (E & awsd) {
		m_target = m_target + (m_up)* glm::vec3(0.001);
		updateCameraMatrix();
	}
}

void ArcBall::Rotate(float dx, float dy) {
	glm::vec3 right = glm::cross(glm::normalize(m_eye), m_up);
	glm::mat4 rot = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	rot = glm::rotate(rot, dx, m_up);
	rot = glm::rotate(rot, dy, right);

	if(m_rotate_camera_center)
		m_target += m_radius * glm::normalize(m_eye);

	m_eye = rot * glm::vec4(m_eye, 1);
	m_up = rot * glm::vec4(m_up, 1);

	if (m_rotate_camera_center)
		m_target -= m_radius * glm::normalize(m_eye);
	else
		m_target = rot * glm::vec4(m_target, 1);
}

void ArcBall::RotateEyeAxis(float dy) {
	glm::mat4 rot = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	rot = glm::rotate(rot, dy, m_eye);
	m_up = rot * glm::vec4(m_up, 1);
}

void ArcBall::Zoom(float distance) {
	m_radius -= distance;

	if (m_radius < 0)
		m_radius = 0.000001;
}

void ArcBall::Pan(float dx, float dy) {

	glm::vec3  right = glm::cross(m_eye, m_up);
	
	m_target = m_target + (right * dx) + (m_up * dy);
}



