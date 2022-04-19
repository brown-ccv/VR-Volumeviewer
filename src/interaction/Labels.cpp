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
#include "GL/glew.h"
#include "GL/wglew.h"
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
#include "GLMLoader.h"
#include <Model.h>
#include <ShaderProgram.h>


Labels::Labels( ShaderProgram& lines_shader, ShaderProgram& plane_shader):
  m_init_plane_model(false), m_lines_shader_program(lines_shader), m_plane_shader_program(plane_shader), m_plane_model(nullptr)
{

}

Labels::~Labels()
{
  clear();
  std::map<std::string, Texture*>::iterator it;
  for (it = m_texture_cache.begin(); it != m_texture_cache.end(); it++)
  {
    delete it->second;
  }
  delete m_plane_model;
}


void Labels::clear() {
  m_text.clear();
  m_position.clear();
  m_position2.clear();
  m_size.clear();
  m_volume.clear();
}

void Labels::set_parent_directory(std::string& directory)
{
  m_parent_directory = directory;
}

unsigned int Labels::create_line_vba(glm::vec3& start, glm::vec3& end)
{
  unsigned int vba;
  unsigned int vbo;
  glGenVertexArrays(1, &vba);
  glGenBuffers(1, &vbo);

  std::vector<glm::vec3> line;
  line.push_back(start);
  line.push_back(end);

  glBindVertexArray(vba);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(glm::vec3) * 2,
    &line[0],
    GL_STATIC_DRAW
  );
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  m_lines_vba.push_back(vba);
  m_lines_vbo.push_back(vbo);

  return vba;
}

void Labels::add( std::string texture_path, float x, float y, float z, float textPosZ, float size, int volume)
{
  if (!m_init_plane_model)
  {
    std::string plane_obj_path = m_parent_directory+"Resources/Models/plane.obj";
    m_plane_model = GLMLoader::loadObjModel(plane_obj_path);
    m_init_plane_model = true;
  }

  if (m_texture_cache.find(texture_path) == m_texture_cache.end())
  {
    
    Texture* texture = new Texture(GL_TEXTURE_2D, texture_path);
    m_texture_cache[texture_path] = texture;
  }
  

  LabelBillboard billboard;
  glm::vec3 line_start(x, y, z);
  glm::vec3 line_end(x, y, textPosZ + 200);
  unsigned int line_vba = create_line_vba(line_start, line_end);
  billboard.line_vba = line_vba;
  billboard.label_texture = m_texture_cache[texture_path];
  billboard.label_model = m_plane_model;
  billboard.position = glm::vec3(x, y, textPosZ+250);
  m_billboard_labels.push_back(billboard);
  
  m_position.push_back(glm::vec3(x, y, z));
  m_position2.push_back(glm::vec3(x, y, textPosZ));
  m_size.push_back(size);
  m_volume.push_back(volume);
}

void Labels::drawLabels(glm::mat4 volume_mv, glm::mat4 projection_matrix, glm::mat4& headpose, float z_scale)
{
  for (int i = 0; i < m_billboard_labels.size(); i++)
  {
    //draw line
    m_lines_shader_program.start();
    m_lines_shader_program.setUniform("p", projection_matrix);
    m_lines_shader_program.setUniform("mv", volume_mv);
    glLineWidth(2);
    glBindVertexArray(m_billboard_labels[i].line_vba);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    m_lines_shader_program.stop();

    glm::vec4 markerpos = volume_mv * glm::vec4(m_billboard_labels[i].position.x, m_billboard_labels[i].position.y, 1, 1);
    glm::vec4 headpos = headpose * glm::vec4(0, 0, 0, 1);
    glm::vec4 dir = headpos / headpos.w - markerpos / markerpos.w;
    dir.w = 0;
    dir = inverse(volume_mv) * dir;
    dir.z = 0;
    dir = normalize(dir);
    float angle = 180.0f / M_PI * atan2(dir.x, dir.y);
    glm::mat4 label_mv;

    label_mv = glm::translate(volume_mv, m_billboard_labels[i].position);
    label_mv = glm::scale(label_mv, glm::vec3(50.0f, 50.0f, 100.0f));
    label_mv = glm::rotate(label_mv, glm::radians(180.0f - angle),glm::vec3(0.0f, 0.0f, 1.0f));
    label_mv = glm::rotate(label_mv, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //label_mv = glm::scale(label_mv, glm::vec3(1.0f, 1.0f / z_scale, 1.0f));
    m_plane_shader_program.start();
    m_plane_shader_program.setUniform("p", projection_matrix);
    m_plane_model->setMVMatrix(label_mv);
    m_billboard_labels[i].label_model->setTexture(m_billboard_labels[i].label_texture);
    m_billboard_labels[i].label_model->render(m_plane_shader_program);
    m_plane_shader_program.stop();
  }
  
}

void Labels::drawLines()
{
  
  for (size_t i =0; i < m_lines_vba.size();++i)
  {
    glBindVertexArray(m_lines_vba[i]);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
  }
  
}
