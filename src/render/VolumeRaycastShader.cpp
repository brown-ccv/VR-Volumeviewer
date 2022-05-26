//  ----------------------------------.

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
///\file VolumeRaycastShader.cpp
///\author Benjamin Knorlein
///\date 11/30/2017

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/VolumeRaycastShader.h"

#include <glm/gtc/type_ptr.hpp>

VolumeRaycastShader::VolumeRaycastShader() : m_use_blending{false}, m_blend_volume{0}, m_blending_alpha{0}, m_clip_min{0.0}, m_clip_max{1.0}
{

  m_shader = "VolumeRaycastShader";

  m_vertexShader = "#version 330 core\n"
                   "layout(location = 0) in vec3 vVertex;\n" // object space vertex position
                   "uniform mat4 MVP; \n"                    // combined modelview projection matrix
                   "smooth out vec3 vUV; \n"                 // 3D texture coordinates for texture lookup in the fragment shader
                   "void main()\n"
                   "{\n"
                   // get the clipspace position
                   "gl_Position = MVP*vec4(vVertex, 1); \n"
                   // get the 3D texture coordinates by adding (0.5,0.5,0.5) to the object space
                   // vertex position. Since the unit cube is at origin (min: (-0.5,-0.5,-0.5) and max: (0.5,0.5,0.5))
                   // adding (0.5,0.5,0.5) to the unit cube object space position gives us values from (0,0,0) to
                   //(1,1,1)
                   "vUV = vVertex + vec3(0.5); \n"
                   "}\n";

  m_fragmentShader =
      "#version 330 core\n"
      "vec2 intersect_box(vec3 orig, vec3 dir, vec3 clip_min, vec3 clip_max) { \n"
      "dir += vec3(0.0000001); \n"
      "vec3 inv_dir = 1.0 / dir; \n"
      "vec3 tmin_tmp = (clip_min - orig) * inv_dir; \n"
      "vec3 tmax_tmp = (clip_max - orig) * inv_dir; \n"
      "vec3 tmin = min(tmin_tmp, tmax_tmp); \n"
      "vec3 tmax = max(tmin_tmp, tmax_tmp); \n"
      "float t0 = max(tmin.x, max(tmin.y, tmin.z)); \n"
      "float t1 = min(tmax.x, min(tmax.y, tmax.z)); \n"
      "return vec2(t0, t1); \n"
      "}\n"

      "layout(location = 0) out vec4 vFragColor; \n" // fragment shader output
      "smooth in vec3 vUV; \n"                       // 3D texture coordinates form vertex shader interpolated by rasterizer
      "uniform sampler3D volume;\n"                  // volume dataset
      "uniform mat4 clipPlane; \n"
      "uniform bool clipping;\n"
      "uniform float threshold;\n"
      "uniform float multiplier;\n"
      "uniform vec3 camPos;\n"          // camera position
      "uniform vec3 step_size;\n"       // ray step size
      "const int MAX_SAMPLES = 3000;\n" // total samples for each ray march step
      "uniform int channel;\n"
      "uniform sampler2D lut;\n" // transferfunction
      "uniform bool useLut;\n"
      "uniform bool useMultiLut;\n"
      "uniform sampler2D depth;\n"
      "uniform vec2 viewport;\n"
      "uniform mat4 P_inv; \n"
      "uniform bool useBlend;\n"
      "uniform sampler3D blendVolume;\n" // volume dataset
      "uniform float blendAlpha;\n"
      "uniform vec3 clip_min;\n"
      "uniform vec3 clip_max;\n"
      "void main()\n"
      "{\n"
      // get the 3D texture coordinates for lookup into the volume dataset
      "vec3 dataPos = vUV; \n"

      // get the object space position by subracting 0.5 from the
      // 3D texture coordinates. Then subtraact it from camera position
      // and normalize to get the ray marching direction
      "vec3 geomDir = normalize( dataPos - camPos); \n"

      // get the t values for the intersection with the box"
      "vec2 t_hit = intersect_box(camPos, geomDir,clip_min,clip_max); \n"

      // We don't want to sample voxels behind the eye if it's
      // inside the volume, so keep the starting point at or in front
      // of the eye
      "if(t_hit.x < 0.0f) t_hit.x= max(t_hit.x, 0.0); \n"

      // We not know if the ray was cast from the back or the front face. (Note: For now we also render the back face only)
      // To ensure we update dataPos and t_hit to reflect a ray from entry point to exit
      "dataPos = camPos + t_hit.x * geomDir;\n"
      "t_hit.y = t_hit.y-t_hit.x; \n"
      "t_hit.x = 0.0001f; \n"

      // get t for the clipping plane and overwrite the entry point
      "if(clipping){ \n"
      "vec4 p_in = clipPlane * vec4(dataPos + t_hit.x * geomDir, 1);\n"
      "vec4 p_out = clipPlane * vec4(dataPos + t_hit.y * geomDir, 1);\n"
      "if(p_in.y * p_out.y < 0.0f ){\n"
      // both points lie on different sides of the plane
      // we need to compute a new clippoint
      "vec4 c_pos = clipPlane * vec4(dataPos, 1);\n"
      "vec4 c_dir = clipPlane * vec4(geomDir, 0);\n"
      "float t_clip = -c_pos.y / c_dir.y  ;\n"
      // update either entry or exit based on which is on the clipped side
      "if (p_in.y > 0.0f){\n"
      "t_hit.x = t_clip; \n"
      "}else{\n"
      "t_hit.y = t_clip; \n"
      "}\n"
      "}else{\n"
      // both points lie on the same side of the plane.
      // if one of them is on the wrong side they can be clipped
      "if(p_in.y > 0.0f)\n"
      "discard;\n"
      "}\n"
      "}\n"

      // Compute occlusion point in volume coordinates
      "float d = texture(depth, vec2(gl_FragCoord.x/viewport.x,gl_FragCoord.y/viewport.y)).r; \n"
      "vec4 d_ndc = vec4((gl_FragCoord.x / viewport.x - 0.5) * 2.0,(gl_FragCoord.y / viewport.y - 0.5) * 2.0, (d - 0.5) * 2.0, 1.0); \n"
      "d_ndc = P_inv * d_ndc; \n "
      "d_ndc = d_ndc / d_ndc.w; \n"

      // compute t_occ and check if it closer than the exit point
      /*"float t_occ = ((d_ndc.x + 0.5) - dataPos.x) / geomDir.x; \n"
      "t_hit.y = min(t_hit.y, t_occ); \n"*/

      // first value should always be lower by definition and this case should never occur. If it does discard the fragment.
      "if (t_hit.x > t_hit.y) \n"
      "discard; \n"

      // compute step size as the minimum of the stepsize
      "float dt = min(step_size.x, min(step_size.y, step_size.z)) ;\n"

      // Step 4: Starting from the entry point, march the ray through the volume
      // and sample it
      "dataPos = dataPos + t_hit.x * geomDir; \n"
      "for (float step = t_hit.x; step < t_hit.y; step += dt) {\n"
      // data fetching from the red channel of volume texture
      "vec4 sample; \n"
      "if (channel == 1){ \n"
      "sample = texture(volume, dataPos).rrrr; \n"
      "}else if (channel == 2){ \n"
      "sample = texture(volume, dataPos).gggg; \n"
      "}else if (channel == 3){ \n"
      "sample = texture(volume, dataPos).bbbb; \n"
      "}else if (channel == 4){ \n"
      "sample = texture(volume, dataPos).aaaa; \n"
      "}else if (channel == 5){ \n"
      "sample = texture(volume, dataPos); \n"
      "}else{ \n"
      "sample = texture(volume, dataPos); \n"
      "sample.a = max(sample.r, max(sample.g,sample.b)) ; "
      "}\n"

      "if(useBlend){ \n"
      "vec4 sample_blend; \n"
      "if (channel == 1){ \n"
      "sample_blend = texture(blendVolume, dataPos).rrrr; \n"
      "}else if (channel == 2){ \n"
      "sample_blend = texture(blendVolume, dataPos).gggg; \n"
      "}else if (channel == 3){ \n"
      "sample_blend = texture(blendVolume, dataPos).bbbb; \n"
      "}else if (channel == 4){ \n"
      "sample_blend = texture(blendVolume, dataPos).aaaa; \n"
      "}else if (channel == 5){ \n"
      "sample_blend = texture(blendVolume, dataPos); \n"
      "}else{ \n"
      "sample_blend = texture(blendVolume, dataPos); \n"
      "sample_blend.a = max(sample_blend.r, max(sample_blend.g,sample_blend.b)) ; "
      "}\n"

      "sample = (vec4(1.0f - blendAlpha) * sample) +  (vec4(blendAlpha) * sample_blend);\n"
      "}\n"

      // threshold based on alpha
      "sample.a = (sample.a > threshold) ? sample.a : 0.0f ;\n"

      // transferfunction
      "if(useLut) {\n"
      "if(useMultiLut){\n"
      "sample.r = texture(lut, vec2(sample.r,0.5)).r;"
      "sample.g = texture(lut, vec2(sample.g,0.5)).g;"
      "sample.b = texture(lut, vec2(sample.b,0.5)).b;"
      "sample.a = max(sample.r, max(sample.g,sample.b)) ; "
      "}else{\n"
      "sample = texture(lut, vec2(clamp(sample.a,0.0,1.0),0.5));"
      "}\n"
      "}\n"

      // assume alpha is the highest channel and gamma correction
      "sample.a = sample.a * multiplier; \n" /// needs changing

      // blending (front to back)
      "vFragColor.rgb += (1.0 - vFragColor.a) * sample.a * sample.rgb;\n"
      "vFragColor.a += (1.0 - vFragColor.a) * sample.a;\n"

      // early exit if opacity is reached
      "if (vFragColor.a >= 0.95) \n"
      "break;\n"

      // advance point
      "dataPos += geomDir * dt; \n"
      "} \n"

      // remove fragments for correct depthbuffer
      "if (vFragColor.a < 0.001f)"
      "discard;"
      "}\n";
}

VolumeRaycastShader::~VolumeRaycastShader()
{
}

void VolumeRaycastShader::render(glm::mat4 &MVP, glm::mat4 &clipPlane, glm::vec3 &camPos)
{
  if (m_use_blending)
  {
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_3D, m_blend_volume);
  }

  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, m_depth_texture);

  bindProgram();

  ////pass the shader uniform
  glUniformMatrix4fv(m_MVP_uniform, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(m_clipPlane_uniform, 1, GL_FALSE, glm::value_ptr(clipPlane));
  glUniform3f(m_camPos_uniform, camPos.x, camPos.y, camPos.z);
  glUniform3f(m_step_size_uniform, m_stepSize[0], m_stepSize[1], m_stepSize[2]);
  glUniform1f(m_threshold_uniform, m_threshold);
  glUniform1f(m_multiplier_uniform, m_multiplier);
  glUniform1i(m_clipping_uniform, m_clipping);
  glUniform1i(m_channel_uniform, m_channel);
  glUniform1i(m_useLut_uniform, m_useLut);
  glUniform1i(m_useMultiLut_uniform, m_useMultiLut);
  glUniform2f(m_viewport_uniform, m_screen_size[0], m_screen_size[1]);
  glUniformMatrix4fv(m_P_inv_uniform, 1, GL_FALSE, glm::value_ptr(m_P_inv));
  glUniform1i(m_useBlend_uniform, m_use_blending);
  glUniform1f(m_blendAlpha_uniform, m_blending_alpha);
  glUniform3f(m_clip_min_uniform, m_clip_min.x, m_clip_min.y, m_clip_min.z);
  glUniform3f(m_clip_max_uniform, m_clip_max.x, m_clip_max.y, m_clip_max.z);

  //////draw the triangles
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
  ////unbind the shader
  unbindProgram();

  glBindTexture(GL_TEXTURE_2D, 0);

  if (m_use_blending)
  {
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_3D, 0);
  }
}

void VolumeRaycastShader::initGL()
{

  bindProgram();

  // add attributes and uniforms
  m_volume_uniform = glGetUniformLocation(m_programID, "volume");
  m_MVP_uniform = glGetUniformLocation(m_programID, "MVP");
  m_clipPlane_uniform = glGetUniformLocation(m_programID, "clipPlane");
  m_vVertex_attribute = glGetAttribLocation(m_programID, "vVertex");
  m_camPos_uniform = glGetUniformLocation(m_programID, "camPos");
  m_step_size_uniform = glGetUniformLocation(m_programID, "step_size");
  m_threshold_uniform = glGetUniformLocation(m_programID, "threshold");
  m_multiplier_uniform = glGetUniformLocation(m_programID, "multiplier");
  m_clipping_uniform = glGetUniformLocation(m_programID, "clipping");
  m_channel_uniform = glGetUniformLocation(m_programID, "channel");
  m_lut_uniform = glGetUniformLocation(m_programID, "lut");
  m_useLut_uniform = glGetUniformLocation(m_programID, "useLut");
  m_useMultiLut_uniform = glGetUniformLocation(m_programID, "useMultiLut");
  m_viewport_uniform = glGetUniformLocation(m_programID, "viewport");
  m_depth_uniform = glGetUniformLocation(m_programID, "depth");
  m_P_inv_uniform = glGetUniformLocation(m_programID, "P_inv");
  m_viewport_uniform = glGetUniformLocation(m_programID, "viewport");
  m_depth_uniform = glGetUniformLocation(m_programID, "depth");
  m_P_inv_uniform = glGetUniformLocation(m_programID, "P_inv");
  m_useBlend_uniform = glGetUniformLocation(m_programID, "useBlend");
  m_blendAlpha_uniform = glGetUniformLocation(m_programID, "blendAlpha");
  m_blendVolume_uniform = glGetUniformLocation(m_programID, "blendVolume");
  m_clip_min_uniform = glGetUniformLocation(m_programID, "clip_min");
  m_clip_max_uniform = glGetUniformLocation(m_programID, "clip_max");

  ////pass constant uniforms at initialization
  glUniform1i(m_volume_uniform, 0);
  glUniform1i(m_lut_uniform, 1);
  glUniform1i(m_depth_uniform, 2);
  glUniform1i(m_blendVolume_uniform, 3);

  unbindProgram();
}
