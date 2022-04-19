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
///\file ArcBall.h
///\author Benjamin Knorlein
///\date 10/17/2019

#ifndef ARCBALL_H
#define ARCBALL_H

#include <glm/glm.hpp>
#include <string>
#include <list>

#include "choreograph/Choreograph.h"

enum WASD_KEYS
{
  W = 1 << 0, // binary 000001
  A = 1 << 1, // binary 000010
  S = 1 << 2, // binary 000100
  D = 1 << 3, // binary 001000
  Q = 1 << 4, // binary 010000
  E = 1 << 5, // binary 100000
};

struct PointOfInterest {
  
 
  glm::vec3 eye = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  float radius = 1.f;
  glm::vec3 max_clip;
  glm::vec3 min_clip;

  std::string label;

  glm::vec3 get_camera_position()
  {
    eye = glm::normalize(eye);
    return radius * eye + target;
  }

};

enum CAMERA_ANIMATION_STATE
{
  STOP,
  PLAYING,
  PAUSE
};

/** Adds a HeadMatrix to the RenderState that gets updated repeatedly based
    upon head tracking events.
 */

class VRVolumeApp;
class ArcBallCamera {
public:

  ArcBallCamera();

  virtual ~ArcBallCamera();

  void mouse_pressed(int button, bool isDown);
  void mouse_move(float x, float y);
  void mouse_scroll(float dist);
  void setCameraCenterRotation(bool useCameraCenter);
  void wasd_pressed(int awsd);

  glm::mat4& getViewmatrix()
  {
    updateCameraMatrix();
    return m_viewmatrix;
  }

  std::list<PointOfInterest>& get_camera_poi();

  void add_camera_poi(std::string & label, glm::vec3& clip_max, glm::vec3& clip_min);
  void add_camera_poi(std::string& label, float eye_x, float eye_y, float eye_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z, float radius);

  int get_current_poi();

  void set_current_poi(int val);

  void remove_poi(int val);

  void reset_camera();

  PointOfInterest& get_poi_at(int val);

  void update_animation();

  void set_animation_path();

  void set_animation_state();

  CAMERA_ANIMATION_STATE get_animation_state();

  std::string get_camera_animation_state();

  float get_camera_animation_duration();
  void set_camera_animation_duration(float duration);

  void set_controller_application(VRVolumeApp* vr_app);

protected:
  void Rotate(float dTheta, float dPhi);
  void RotateEyeAxis(float dy);
  void Zoom(float distance);
  void Pan(float dx, float dy);
  void updateCameraMatrix();

  float m_radius;
  glm::vec3 m_target;
  glm::vec3 m_up;
  glm::vec3 m_eye;
  std::list<PointOfInterest> m_camera_poi;
  PointOfInterest m_current_poi;

  glm::mat4 m_viewmatrix;
  bool m_mouse_left_pressed;
  bool m_mouse_right_pressed;
  bool m_mouse_center_pressed;
  float last_x, last_y;
  float m_PanFactor;
  float m_RotateFactor;
  float m_cameraScrollFactor;

  bool m_rotate_camera_center;
  ch::Output<glm::vec3>  m_target_animation;
  ch::Output<glm::vec3>  m_eye_animation;
  ch::Output<glm::vec3>  m_up_animation;
  ch::Output<float>  m_radius_animation;
  ch::Output<glm::vec3>  m_clip_max_animation;
  ch::Output<glm::vec3>  m_clip_min_animation;
  ch::Timeline     m_timeline;

  bool m_is_animate_path;
  float m_animation_duration;
  
  CAMERA_ANIMATION_STATE m_camera_animation_state;
  std::string animation_button_label;

  VRVolumeApp* m_controller_app;
};

#endif

