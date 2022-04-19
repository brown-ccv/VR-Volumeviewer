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


#include "../../include/interaction/ArcBallCamera.h"
#include <glm/glm.hpp>
#include "glm/ext.hpp"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "../../include/vrapp/VRVolumeApp.h"

#ifndef _PI
#define _PI 3.141592653
#endif

ArcBallCamera::ArcBallCamera() : m_radius(1), m_mouse_left_pressed(false), m_mouse_center_pressed(false), m_mouse_right_pressed(false), last_x(0), last_y(0)
, m_PanFactor(1), m_RotateFactor(1), m_cameraScrollFactor(0.1), m_target(0, 0, 0), m_eye(0, 0, 1), m_up(0, 1, 0), m_rotate_camera_center{ false }
, m_is_animate_path(false), m_camera_animation_state(STOP), animation_button_label("ANIMATE"), m_animation_duration(10.f)
{

}

ArcBallCamera::~ArcBallCamera()
{

}

void ArcBallCamera::updateCameraMatrix()
{
  if (m_is_animate_path)
  {
    /*glm::vec3 animation_position = m_animation;
    viewmatrix =  glm::lookAt(animation_position, m_current_poi.target, m_current_poi.up);*/
    if (m_camera_animation_state != PAUSE)
    {
      update_animation();
      m_controller_app->set_clip_max(m_clip_max_animation.value());
      m_controller_app->set_clip_min(m_clip_min_animation.value());

    }
    if (m_timeline.isFinished())
    {
      m_camera_animation_state = STOP;
      animation_button_label = "Animate";
      m_timeline.resetTime();
      if (m_controller_app->get_movie_state() == MOVIE_RECORD)
      {
        m_controller_app->stop_movie();
      }
      
    }


    PointOfInterest poi { m_eye_animation.value(), m_target_animation.value(), m_up_animation.value(), m_radius_animation.value() };
    m_viewmatrix = glm::lookAt(poi.get_camera_position(), poi.target, poi.up);


  }
  else
  {
    m_viewmatrix = glm::lookAt(m_current_poi.get_camera_position(), m_current_poi.target, m_current_poi.up);
  }

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

  std::cout << m_current_poi.radius << std::endl;
}

void ArcBallCamera::Pan(float dx, float dy) {

  glm::vec3  right = glm::cross(m_current_poi.eye, m_current_poi.up);

  m_current_poi.target = m_current_poi.target + (right * dx) + (m_current_poi.up * dy);
}

std::list<PointOfInterest>& ArcBallCamera::get_camera_poi()
{
  return m_camera_poi;
}

void ArcBallCamera::add_camera_poi(std::string& label, glm::vec3& clip_max, glm::vec3& clip_min)
{
  m_current_poi.label = label;
  m_current_poi.max_clip = clip_max;
  m_current_poi.min_clip = clip_min;

  m_camera_poi.push_back(m_current_poi);
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
  m_is_animate_path = false;
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

void ArcBallCamera::reset_camera()
{
  m_is_animate_path = false;
  last_x = 0;
  last_y = 0;
  m_PanFactor = 1;
  m_RotateFactor = 1;
  m_cameraScrollFactor = 0.1;
  m_target = glm::vec3(0, 0, 0);
  m_eye = glm::vec3(0, 0, 1);
  m_up = glm::vec3(0, 1, 0);
  m_current_poi = PointOfInterest();
}

PointOfInterest& ArcBallCamera::get_poi_at(int val)
{
  auto poi_iterator = m_camera_poi.begin();
  std::advance(poi_iterator, val);
  return *poi_iterator;
}

void ArcBallCamera::update_animation()
{
  if (m_is_animate_path)
  {
    m_timeline.step(1.0 / 30.0);

  }


}

void ArcBallCamera::set_animation_path()
{
  if (m_camera_poi.size() > 1)
  {

    PointOfInterest first_position = m_camera_poi.front();

    ch::Sequence<glm::vec3> sequence_eye(first_position.eye);
    ch::Sequence<glm::vec3> sequence_target(first_position.target);
    ch::Sequence<glm::vec3> sequence_up(first_position.up);
    ch::Sequence<float> sequence_radius(first_position.radius);
    ch::Sequence<glm::vec3> sequence_clip_max(first_position.max_clip);
    ch::Sequence<glm::vec3> sequence_clip_min(first_position.min_clip);

    
    for (auto next_camera_poi_interator = std::next(m_camera_poi.begin()); next_camera_poi_interator != m_camera_poi.end(); next_camera_poi_interator++)
    {
      sequence_eye.then<ch::RampTo>(next_camera_poi_interator->eye, m_animation_duration);
      sequence_target.then<ch::RampTo>(next_camera_poi_interator->target, m_animation_duration);
      sequence_up.then<ch::RampTo>(next_camera_poi_interator->up, m_animation_duration);
      sequence_radius.then<ch::RampTo>(next_camera_poi_interator->radius, m_animation_duration);
      sequence_clip_max.then<ch::RampTo>(next_camera_poi_interator->max_clip, m_animation_duration);
      sequence_clip_min.then<ch::RampTo>(next_camera_poi_interator->min_clip, m_animation_duration);
    }


    auto group = std::make_shared<ch::Timeline>();
    group->apply<glm::vec3>(&m_eye_animation, sequence_eye);
    group->apply<glm::vec3>(&m_target_animation, sequence_target);
    group->apply<glm::vec3>(&m_up_animation, sequence_up);
    group->apply<float>(&m_radius_animation, sequence_radius);
    group->apply<glm::vec3>(&m_clip_max_animation, sequence_clip_max);
    group->apply<glm::vec3>(&m_clip_min_animation, sequence_clip_min);

    m_timeline.addShared(group);

  }

}

void ArcBallCamera::set_animation_state()
{
  if (m_camera_animation_state == STOP)
  {
    set_animation_path();
    m_camera_animation_state = PLAYING;
    animation_button_label = "PAUSE";
  }
  else if (m_camera_animation_state == PLAYING)
  {
    if (!m_timeline.isFinished())
    {
      m_camera_animation_state = PAUSE;
      animation_button_label = "ANIMATE";
    }
  }
  else if (m_camera_animation_state == PAUSE)
  {
    m_camera_animation_state = PLAYING;
    animation_button_label = "PAUSE";
  }

  m_is_animate_path = true;

}

CAMERA_ANIMATION_STATE ArcBallCamera::get_animation_state()
{
  if (m_timeline.isFinished())
  {
    return STOP;
  }
  else if (!m_timeline.isFinished() && m_camera_animation_state == PLAYING)
  {
    return  PLAYING;
  }
  else if (!m_timeline.isFinished() && m_camera_animation_state == STOP)
  {
    return  PAUSE;
  }
}

std::string ArcBallCamera::get_camera_animation_state()
{
  return animation_button_label;
}

float ArcBallCamera::get_camera_animation_duration()
{
  return m_animation_duration;
}

void ArcBallCamera::set_camera_animation_duration(float duration)
{
  m_animation_duration = duration;
}

void ArcBallCamera::set_controller_application(VRVolumeApp* vr_app)
{
  m_controller_app = vr_app;
}

