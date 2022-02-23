//  ----------------------------------
//  Copyright � 2017, Brown University, Providence, RI.
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
//  PROVIDED �AS IS�, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
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


#include "../../include/interaction/ArcBallCamera.h"
#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#ifndef _PI
#define _PI 3.141592653
#endif

ArcBallCamera::ArcBallCamera() : m_radius(1), m_mouse_left_pressed(false), m_mouse_center_pressed(false), m_mouse_right_pressed(false), last_x(0), last_y(0)
, m_PanFactor(1), m_RotateFactor(1), m_cameraScrollFactor(0.1), m_target(0, 0, 0), m_eye(0, 0, 1), m_up(0, 1, 0), m_rotate_camera_center{ false }
{
  
}

ArcBallCamera::~ArcBallCamera()
{

}

void ArcBallCamera::updateCameraMatrix()
{
  viewmatrix = glm::lookAt(m_current_poi.get_camera_position(), m_current_poi.target, m_current_poi.up);
}

void ArcBallCamera::mouse_pressed(int button, bool isDown)
{
  if (button == 0) //left -> rotate
  {
    m_mouse_left_pressed = isDown;
  }
  else if (button == 1) // right ->pan
  {
    m_mouse_right_pressed = isDown;

  }
  else if (button == 2)
  {

    m_mouse_center_pressed = isDown;
  }
}

void ArcBallCamera::mouse_move(float x, float y) {
  if (m_mouse_left_pressed) {
    // Calculate the new phi and theta based on mouse position relative to where the user clicked
    float dx = ((float)(last_x - x)) / 300.0f;
    float dy = ((float)(last_y - y)) / 300.0f;

    Rotate(dx * m_RotateFactor, -dy * m_RotateFactor);
  }
  else if (m_mouse_center_pressed) {
    float dy = ((float)(last_y - y)) / 300.0f;

    RotateEyeAxis(dy * m_RotateFactor);
  }
  else if (m_mouse_right_pressed) {
    float dx = ((float)(last_x - x)) / 300.0f;
    float dy = ((float)(last_y - y)) / 300.0f;

    Pan(-dx * m_PanFactor, -dy * m_PanFactor);
  }

  last_x = x;
  last_y = y;
}

void ArcBallCamera::mouse_scroll(float dist) {
  Zoom(dist);
}

void ArcBallCamera::setCameraCenterRotation(bool useCameraCenter) {
  if (useCameraCenter != m_rotate_camera_center) {
    m_rotate_camera_center = useCameraCenter;
    if (!m_rotate_camera_center) {
      m_current_poi.target = glm::vec3{ 0.0 };
    }
  }
}

void ArcBallCamera::wasd_pressed(int awsd) {
  glm::vec3 dir = glm::normalize(-m_current_poi.eye);
  glm::vec3  right = glm::cross(dir, m_current_poi.up);

  if (W & awsd) {
    m_current_poi.target = m_current_poi.target + dir * glm::vec3(0.001);
    updateCameraMatrix();
  }
  if (S & awsd) {
    m_current_poi.target = m_current_poi.target - dir * glm::vec3(0.001);
    updateCameraMatrix();
  }
  if (A & awsd) {
    m_current_poi.target = m_current_poi.target - (right)*glm::vec3(0.001);
    updateCameraMatrix();
  }
  if (D & awsd) {
    m_current_poi.target = m_current_poi.target + (right)*glm::vec3(0.001);
    updateCameraMatrix();
  }
  if (Q & awsd) {
    m_current_poi.target = m_target - m_current_poi.up * glm::vec3(0.001);
    updateCameraMatrix();
  }
  if (E & awsd) {
    m_current_poi.target = m_target + m_current_poi.up * glm::vec3(0.001);
    updateCameraMatrix();
  }
}

void ArcBallCamera::Rotate(float dx, float dy) {
  glm::vec3 right = glm::cross(glm::normalize(m_current_poi.eye), m_current_poi.up);
  glm::mat4 rot = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  rot = glm::rotate(rot, dx, m_current_poi.up);
  rot = glm::rotate(rot, dy, right);

  if (m_rotate_camera_center)
    m_current_poi.target += m_current_poi.radius * glm::normalize(m_current_poi.eye);

  m_current_poi.eye = rot * glm::vec4(m_current_poi.eye, 1);
  m_current_poi.up = rot * glm::vec4(m_current_poi.up, 1);

  if (m_rotate_camera_center)
    m_current_poi.target -= m_current_poi.radius * glm::normalize(m_current_poi.eye);
  else
    m_current_poi.target = rot * glm::vec4(m_current_poi.target, 1);
}

void ArcBallCamera::RotateEyeAxis(float dy) {
  glm::mat4 rot = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  rot = glm::rotate(rot, dy, m_current_poi.eye);
  m_current_poi.up = rot * glm::vec4(m_current_poi.up, 1);
}

void ArcBallCamera::Zoom(float distance) {
  m_current_poi.radius -= distance;

  if (m_current_poi.radius < 0)
  {
    m_current_poi.radius = 0.000001;
  }
   
}

void ArcBallCamera::Pan(float dx, float dy) {

  glm::vec3  right = glm::cross(m_current_poi.eye, m_current_poi.up);

  m_current_poi.target = m_current_poi.target + (right * dx) + (m_current_poi.up * dy);
}

std::list<PointOfInterest>& ArcBallCamera::get_camera_poi()
{
  return m_camera_poi;
}

void ArcBallCamera::add_camera_poi()
{
  m_current_poi.label = "Position-" + std::to_string(m_camera_poi.size() + 1);
  m_camera_poi.push_front(m_current_poi);
}

void ArcBallCamera::add_camera_poi(std::string& label, float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z, float radius)
{
  PointOfInterest poi;
  poi.label = label;
  poi.eye = glm::vec3(eye_x, eye_y, eye_z);
  poi.target = glm::vec3(target_x, target_y, target_z);
  poi.up = glm::vec3(up_x, up_y, up_z);
  poi.radius = radius;
  m_camera_poi.push_front(poi);
}

int ArcBallCamera::get_current_poi()
{ 
  return 0; 
}

void ArcBallCamera::set_current_poi(int val) 
{ 
  std::list<PointOfInterest>::iterator it = m_camera_poi.begin();
  for (int i = 0; i < val; i++) {
    ++it;
  }
  m_current_poi = *it;
}

void ArcBallCamera::remove_poi(int val)
{
  auto poi_iterator = m_camera_poi.begin();
  std::advance(poi_iterator, val);
  m_camera_poi.erase(poi_iterator);
}

void ArcBallCamera::rest_camera()
{
  m_current_poi = PointOfInterest();
}

PointOfInterest& ArcBallCamera::get_poi_at(int val)
{
  auto poi_iterator = m_camera_poi.begin();
  std::advance(poi_iterator, val);
  return *poi_iterator;
}

