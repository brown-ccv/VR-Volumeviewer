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
///\file VolumeSliceRcastRender.h
///\author Benjamin Knorlein
///\date 05/24/2019
/// Based on the book : OpenGL Development Cookbook  by Muhammad Mobeen Movania

#ifndef VOLUMESLICERCASTRENDER_H
#define VOLUMESLICERCASTRENDER_H

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include "VolumeRenderer.h"
#include "render/VolumeRaycastShader.h"



class VRVolumeApp;
class VolumeRaycastRenderer : public VolumeRenderer
{
public:
  VolumeRaycastRenderer(VRVolumeApp& volume_app);
  ~VolumeRaycastRenderer();

  virtual void initGL() override;
  virtual void render(Volume *volume, const glm::mat4 &MV, glm::mat4 &P, float z_scale, GLint colormap, int renderChannel) override;

  virtual void set_threshold(float threshold) override;
  virtual void set_multiplier(float multiplier) override;
  virtual void set_blending(bool useBlending, float alpha, Volume *volume) override;
  virtual void useMultichannelColormap(bool useMulti);

  virtual void set_num_slices(int slices) override;

  void setDepthTexture(DepthTexture *depth_texture)
  {
    shader.setDepthTexture(depth_texture);
  }

  virtual void set_clip_min_max(glm::vec3 min_clip, glm::vec3 max_clip);

private:
  void setChannel(Volume *volume);

  ////volume vertex array and buffer objects
  GLuint cubeVBOID;
  GLuint cubeVAOID;
  GLuint cubeIndicesID;

  // 3D texture slicing shader
  VolumeRaycastShader shader;
  VRVolumeApp& m_volume_app;

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
};
#endif // VOLUMESLICERCASTRENDER_H
