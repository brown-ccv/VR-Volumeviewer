﻿//  ----------------------------------
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
///\file VolumeRaycastShader.h
///\author Benjamin Knorlein
///\date 05/24/2019
/// Based on the book : OpenGL Development Cookbook  by Muhammad Mobeen Movania

#ifndef VOLUMERAYCASTSHADER_H
#define VOLUMERAYCASTSHADER_H

#include "Shader.h"

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include "DepthTexture.h"
#include <iostream>
#include "ShaderProgram.h"

class VolumeRaycastShader : public Shader
{

public:
  VolumeRaycastShader();
  virtual ~VolumeRaycastShader();

  void render(glm::mat4 &MVP, glm::mat4 &clipPlane, glm::vec3 &camPos);
  void initGL(std::string& shader_file_path);

  void set_stepSize(float x, float y, float z)
  {
    m_stepSize[0] = x;
    m_stepSize[1] = y;
    m_stepSize[2] = z;
  }

  void set_threshold(float threshold)
  {
    m_threshold = threshold;
  }

  void set_multiplier(float multiplier)
  {
    m_multiplier = multiplier;
  }

  void set_clipping(bool clipping)
  {
    m_clipping = clipping;
  }

  void set_channel(int channel)
  {
    m_channel = channel;
  }

  void set_useLut(bool useLUT)
  {
    m_useLut = useLUT;
  }

  void setDepthTexture(DepthTexture *depth_texture)
  {
    m_depth_texture = depth_texture->depth_texture();
    m_screen_size[0] = depth_texture->width();
    m_screen_size[1] = depth_texture->height();
    m_buffer_size[0] = depth_texture->frame_buffer_width();
    m_buffer_size[1] = depth_texture->frame_buffer_height();
    m_display_scale[0] = depth_texture->display_scale_x();
    m_display_scale[1] = depth_texture->display_scale_y();
  }

  void set_P_inv(glm::mat4 &P_inv)
  {
    m_P_inv = P_inv;
  }

  void set_blending(bool useBlending, float alpha, unsigned int texID)
  {
    m_use_blending = useBlending;
    m_blend_volume = texID;
    m_blending_alpha = alpha;
  }

  void useMultichannelColormap(bool useMultiLut)
  {
    m_useMultiLut = useMultiLut;
  }

  void setClipping(glm::vec3 min_clip, glm::vec3 max_clip)
  {
    m_clip_min = min_clip;
    m_clip_max = max_clip;
  }

  void setNumSlices(float slices)
  {
    m_slices = slices;
  }

  void setNumDim(float dim)
  {
    m_dim = dim;
  }

private:
  GLuint m_volume_uniform;
  GLuint m_vVertex_attribute;
  GLuint m_MVP_uniform;

  GLuint m_camPos_uniform;

  float m_stepSize[3];
  GLuint m_step_size_uniform;

  bool m_clipping;
  GLuint m_clipping_uniform;
  GLuint m_clipPlane_uniform;

  float m_threshold;
  float m_multiplier;
  int m_channel;
  float m_slices;
  float m_dim;

  GLuint m_threshold_uniform;
  GLuint m_multiplier_uniform;
  GLuint m_channel_uniform;

  bool m_useLut;
  bool m_useMultiLut;
  GLuint m_lut_uniform;
  GLint m_useLut_uniform;
  GLuint m_useMultiLut_uniform;

  unsigned int m_depth_texture;
  unsigned int m_screen_size[2];
  unsigned int m_buffer_size[2];
  unsigned int m_display_scale[2];

  glm::mat4 m_P_inv;
  GLuint m_depth_uniform;
  GLuint m_viewport_uniform;
  GLuint m_P_inv_uniform;

  bool m_use_blending;
  unsigned int m_blend_volume;
  float m_blending_alpha;
  GLuint m_useBlend_uniform;
  GLuint m_blendAlpha_uniform;
  GLuint m_blendVolume_uniform;

  GLuint m_clip_min_uniform;
  GLuint m_clip_max_uniform;
  GLuint m_frambuffer_uniform;
  GLuint m_display_scale_uniform;
  GLuint m_slices_uniform;
  GLuint m_dim_uniform;
  glm::vec3 m_clip_min;
  glm::vec3 m_clip_max;

  ShaderProgram m_shader_program;
};
#endif // VOLUMERAYCASTSHADER_H
