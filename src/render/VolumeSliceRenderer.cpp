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
///\file VolumeSliceRender.cpp
///\author Benjamin Knorlein
///\date 11/29/2017
/// Based on the book : OpenGL Development Cookbook  by Muhammad Mobeen Movania



#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/VolumeSliceRenderer.h"
#include <ostream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

//for floating point inaccuracy
const float EPSILON = 0.0001f;

glm::vec3 VolumeSliceRenderer::vertexList[8] = { glm::vec3(-0.5, -0.5, -0.5),
                        glm::vec3(0.5, -0.5, -0.5),
                        glm::vec3(0.5, 0.5, -0.5),
                        glm::vec3(-0.5, 0.5, -0.5),
                        glm::vec3(-0.5, -0.5, 0.5),
                        glm::vec3(0.5, -0.5, 0.5),
                        glm::vec3(0.5, 0.5, 0.5),
                        glm::vec3(-0.5, 0.5, 0.5) };


////unit cube edges
const int VolumeSliceRenderer::edgeList[8][12] = {
                        { 0, 1, 5, 6, 4, 8, 11, 9, 3, 7, 2, 10 }, // v0 is front
                        { 0, 4, 3, 11, 1, 2, 6, 7, 5, 9, 8, 10 }, // v1 is front
                        { 1, 5, 0, 8, 2, 3, 7, 4, 6, 10, 9, 11 }, // v2 is front
                        { 7, 11, 10, 8, 2, 6, 1, 9, 3, 0, 4, 5 }, // v3 is front
                        { 8, 5, 9, 1, 11, 10, 7, 6, 4, 3, 0, 2 }, // v4 is front
                        { 9, 6, 10, 2, 8, 11, 4, 7, 5, 0, 1, 3 }, // v5 is front
                        { 9, 8, 5, 4, 6, 1, 2, 0, 10, 7, 11, 3 }, // v6 is front
                        { 10, 9, 6, 5, 7, 2, 3, 1, 11, 4, 8, 0 }  // v7 is front
};

const int VolumeSliceRenderer::edges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 } };

VolumeSliceRenderer::VolumeSliceRenderer() : num_slices{ 256 }
{
  for (int i = 0; i < MAX_SLICES * 12; i++) {
    vTextureSlices[i] = glm::vec3(0, 0, 0);
  }
}

VolumeSliceRenderer::~VolumeSliceRenderer()
{
  glDeleteVertexArrays(1, &volumeVAO);
  glDeleteBuffers(1, &volumeVBO);
}

void VolumeSliceRenderer::initGL()
{
  //Load and init the texture slicing shader
  shader.initGL();

  //setup the vertex array and buffer objects
  glGenVertexArrays(1, &volumeVAO);
  glGenBuffers(1, &volumeVBO);

  glBindVertexArray(volumeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);

  //pass the sliced vertices vector to buffer object memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(vTextureSlices), 0, GL_DYNAMIC_DRAW);

  //enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
}

void VolumeSliceRenderer::render(Volume* volume, const glm::mat4& MV, glm::mat4& P, float z_scale, GLint colormap, int renderChannel)
{
  glDepthMask(GL_FALSE);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_3D, volume->get_texture_id());
  if (colormap >= 0)
  {
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, colormap);
    shader.set_useLut(true);
  }
  else
  {
    shader.set_useLut(false);
  }

  //get the current view direction vector
  glm::vec4 tmp = MV * glm::vec4(0, 0, 0, 1);
  tmp[3] = 0;
  tmp = inverse(MV) * tmp;
  glm::vec3 new_viewDir = glm::vec3(tmp[0], tmp[1], tmp[2]);
  glm::normalize(new_viewDir);

  if (new_viewDir != viewDir)
  {
    viewDir = new_viewDir;
    SliceVolume();
  }

  //enable alpha blending (use over operator)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //now do the z_scaling
  glm::mat4 MV_tmp = glm::scale(MV, glm::vec3(1, 1, z_scale));
  //get the combined modelview projection matrix
  glm::mat4 MVP = P * MV_tmp;
  glm::mat4 clipPlane;
  if (m_clipping) {
    clipPlane = glm::translate(m_clipPlane * MV_tmp, glm::vec3(-0.5f));
    shader.set_clipping(true);
  }
  else
  {
    shader.set_clipping(false);
  }

  if (renderChannel == 0)
    setChannel(volume);
  else
    shader.set_channel(renderChannel);

  //bind volume vertex array object
  glBindVertexArray(volumeVAO);
  //use the volume shader
  shader.render(MVP, clipPlane, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));

  //disable blending
  glBindVertexArray(0);
  glDisable(GL_BLEND);

  if (colormap >= 0)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
  }
  glBindTexture(GL_TEXTURE_3D, 0);

  glDepthMask(GL_TRUE);
}

void VolumeSliceRenderer::set_threshold(float threshold)
{
  shader.set_threshold(threshold);
}

void VolumeSliceRenderer::set_multiplier(float multiplier)
{
  shader.set_multiplier(multiplier);
}

void VolumeSliceRenderer::set_numSlices(int slices)
{
  if (slices != num_slices) {
    num_slices = (MAX_SLICES < slices) ? MAX_SLICES : slices;
    SliceVolume();
  }
}

void VolumeSliceRenderer::useMultichannelColormap(bool useMulti)
{
  shader.useMultichannelColormap(useMulti);
}

void VolumeSliceRenderer::setClipMinMax(glm::vec3 min_clip, glm::vec3 max_clip) {

  if (m_clip_min != min_clip || m_clip_max != max_clip) {
    m_clip_min = min_clip;
    m_clip_max = max_clip;

    VolumeSliceRenderer::vertexList[0] = glm::vec3(m_clip_min.x - 0.5, m_clip_min.y - 0.5, m_clip_min.z - 0.5);
    VolumeSliceRenderer::vertexList[1] = glm::vec3(m_clip_max.x - 0.5, m_clip_min.y - 0.5, m_clip_min.z - 0.5);
    VolumeSliceRenderer::vertexList[2] = glm::vec3(m_clip_max.x - 0.5, m_clip_max.y - 0.5, m_clip_min.z - 0.5);
    VolumeSliceRenderer::vertexList[3] = glm::vec3(m_clip_min.x - 0.5, m_clip_max.y - 0.5, m_clip_min.z - 0.5);
    VolumeSliceRenderer::vertexList[4] = glm::vec3(m_clip_min.x - 0.5, m_clip_min.y - 0.5, m_clip_max.z - 0.5);
    VolumeSliceRenderer::vertexList[5] = glm::vec3(m_clip_max.x - 0.5, m_clip_min.y - 0.5, m_clip_max.z - 0.5);
    VolumeSliceRenderer::vertexList[6] = glm::vec3(m_clip_max.x - 0.5, m_clip_max.y - 0.5, m_clip_max.z - 0.5);
    VolumeSliceRenderer::vertexList[7] = glm::vec3(m_clip_min.x - 0.5, m_clip_max.y - 0.5, m_clip_max.z - 0.5);

    SliceVolume();
  }
}

int VolumeSliceRenderer::FindAbsMax(glm::vec3 v)
{
  v = glm::abs(v);
  int max_dim = 0;
  float val = v.x;
  if (v.y > val) {
    val = v.y;
    max_dim = 1;
  }
  if (v.z > val) {
    val = v.z;
    max_dim = 2;
  }
  return max_dim;
}

void VolumeSliceRenderer::SliceVolume()
{
  //get the max and min distance of each vertex of the unit cube
  //in the viewing direction
  float max_dist = glm::dot(viewDir, vertexList[0]);
  float min_dist = max_dist;
  int max_index = 0;
  int count = 0;

  for (int i = 1; i < 8; i++) {
    //get the distance between the current unit cube vertex and 
    //the view vector by dot product
    float dist = glm::dot(viewDir, vertexList[i]);

    //if distance is > max_dist, store the value and index
    if (dist > max_dist) {
      max_dist = dist;
      max_index = i;
    }

    //if distance is < min_dist, store the value 
    if (dist < min_dist)
      min_dist = dist;
  }
  //find tha abs maximum of the view direction vector
  int max_dim = FindAbsMax(viewDir);

  //expand it a little bit
  min_dist -= EPSILON;
  max_dist += EPSILON;

  //local variables to store the start, direction vectors, 
  //lambda intersection values
  glm::vec3 vecStart[12];
  glm::vec3 vecDir[12];
  float lambda[12];
  float lambda_inc[12];
  float denom = 0;

  //set the minimum distance as the plane_dist
  //subtract the max and min distances and divide by the 
  //total number of slices to get the plane increment
  float plane_dist = min_dist;
  float plane_dist_inc = (max_dist - min_dist) / float(num_slices);

  //for all edges
  for (int i = 0; i < 12; i++) {
    //get the start position vertex by table lookup
    vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

    //get the direction by table lookup
    vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

    //do a dot of vecDir with the view direction vector
    denom = glm::dot(vecDir[i], viewDir);

    //determine the plane intersection parameter (lambda) and 
    //plane intersection parameter increment (lambda_inc)
    if (1.0 + denom != 1.0) {
      lambda_inc[i] = plane_dist_inc / denom;
      lambda[i] = (plane_dist - glm::dot(vecStart[i], viewDir)) / denom;
    }
    else {
      lambda[i] = -1.0;
      lambda_inc[i] = 0.0;
    }
  }

  //local variables to store the intesected points
  //note that for a plane and sub intersection, we can have 
  //a minimum of 3 and a maximum of 6 vertex polygon
  glm::vec3 intersection[6];
  float dL[12];

  //loop through all slices
  for (int i = num_slices - 1; i >= 0; i--) {

    //determine the lambda value for all edges
    for (int e = 0; e < 12; e++)
    {
      dL[e] = lambda[e] + i * lambda_inc[e];
    }

    //if the values are between 0-1, we have an intersection at the current edge
    //repeat the same for all 12 edges
    if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
      intersection[0] = vecStart[0] + dL[0] * vecDir[0];
    }
    else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
      intersection[0] = vecStart[1] + dL[1] * vecDir[1];
    }
    else if ((dL[3] >= 0.0) && (dL[3] < 1.0)) {
      intersection[0] = vecStart[3] + dL[3] * vecDir[3];
    }
    else continue;

    if ((dL[2] >= 0.0) && (dL[2] < 1.0)) {
      intersection[1] = vecStart[2] + dL[2] * vecDir[2];
    }
    else if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
      intersection[1] = vecStart[0] + dL[0] * vecDir[0];
    }
    else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
      intersection[1] = vecStart[1] + dL[1] * vecDir[1];
    }
    else {
      intersection[1] = vecStart[3] + dL[3] * vecDir[3];
    }

    if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
      intersection[2] = vecStart[4] + dL[4] * vecDir[4];
    }
    else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
      intersection[2] = vecStart[5] + dL[5] * vecDir[5];
    }
    else {
      intersection[2] = vecStart[7] + dL[7] * vecDir[7];
    }
    if ((dL[6] >= 0.0) && (dL[6] < 1.0)) {
      intersection[3] = vecStart[6] + dL[6] * vecDir[6];
    }
    else if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
      intersection[3] = vecStart[4] + dL[4] * vecDir[4];
    }
    else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
      intersection[3] = vecStart[5] + dL[5] * vecDir[5];
    }
    else {
      intersection[3] = vecStart[7] + dL[7] * vecDir[7];
    }
    if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
      intersection[4] = vecStart[8] + dL[8] * vecDir[8];
    }
    else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
      intersection[4] = vecStart[9] + dL[9] * vecDir[9];
    }
    else {
      intersection[4] = vecStart[11] + dL[11] * vecDir[11];
    }

    if ((dL[10] >= 0.0) && (dL[10] < 1.0)) {
      intersection[5] = vecStart[10] + dL[10] * vecDir[10];
    }
    else if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
      intersection[5] = vecStart[8] + dL[8] * vecDir[8];
    }
    else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
      intersection[5] = vecStart[9] + dL[9] * vecDir[9];
    }
    else {
      intersection[5] = vecStart[11] + dL[11] * vecDir[11];
    }

    //after all 6 possible intersection vertices are obtained,
    //we calculated the proper polygon indices by using indices of a triangular fan
    int indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5 };

    //Using the indices, pass the intersection vertices to the vTextureSlices vector
    for (int j = 0; j < 12; j++)
      vTextureSlices[count++] = intersection[indices[j]];
  }
  /*while (count < num_slices * 12){
    vTextureSlices[count++] = glm::vec3(0,0,0);
  }*/

  //update buffer object with the new vertices
  glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
  //glBufferData(GL_ARRAY_BUFFER, 0, count * 12, &(vTextureSlices[0].x, GL_DYNAMIC_DRAW));
  glBufferData(GL_ARRAY_BUFFER, count * 12, &(vTextureSlices[0].x), GL_DYNAMIC_DRAW);

}

void VolumeSliceRenderer::setChannel(Volume* volume)
{
  if (volume->render_channel() == -1)
  {
    if (volume->get_channels() == 1)
    {
      shader.set_channel(1);
    }
    else if (volume->get_channels() == 3)
    {
      shader.set_channel(-1);
    }
    else if (volume->get_channels() == 4)
    {
      shader.set_channel(5);
    }
  }
  else
  {
    shader.set_channel(volume->render_channel());
  }
}
