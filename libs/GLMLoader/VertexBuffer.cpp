//  ----------------------------------
//  XMALab -- Copyright © 2015, Brown University, Providence, RI.
//  
//  All Rights Reserved
//   
//  Use of the XMALab software is provided under the terms of the GNU General Public License version 3 
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
///\file FrameBuffer.cpp
///\author Benjamin Knorlein
///\date 07/29/2016

#include <GL/glew.h>

#include "VertexBuffer.h"
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif



VertexBuffer::VertexBuffer() : m_initialized(false), m_vao_id(0), m_vbo_id(0), m_nbo_id(0), m_tbo_id(0),
m_ibo_id(), m_data_ready(false), m_num_indices(0)
{

}

VertexBuffer::~VertexBuffer()
{
	if (m_initialized)
	{
		if (m_vbo_id != 0)glDeleteBuffers(1, &m_vbo_id);
		if (m_nbo_id != 0)glDeleteBuffers(1, &m_nbo_id);
		if (m_tbo_id != 0)glDeleteBuffers(1, &m_tbo_id);
		if (m_ibo_id != 0)glDeleteBuffers(1, &m_ibo_id);

		m_initialized = false;
	}
	
}

void VertexBuffer::setData(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& texcoords, const std::vector<unsigned int>& indices)
{

	if (vertices.size()) {
		m_vertices = vertices;
	}

	if (normals.size()) {
		m_normals = normals;
	}

	if (texcoords.size()) {
		m_texcoords = texcoords;
	}

	if (indices.size()) {
		m_num_indices = indices.size();
		m_indices = indices;
	}
	m_data_ready = true;
}

void VertexBuffer::render()
{
	std::lock_guard<std::mutex> guard(mutex);
	if (!m_initialized)
	{
		if (!m_data_ready)
			return;

		setupVBO();
	}

	glBindVertexArray(m_vao_id);
	glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);



}

std::vector<glm::vec3>& VertexBuffer::vertices()
{
	return m_vertices;
}

std::vector<glm::vec3>& VertexBuffer::normals()
{
	return m_normals;
}


std::vector<glm::vec2>& VertexBuffer::texcoords()
{
	return m_texcoords;
}


std::vector<unsigned int>& VertexBuffer::indices()
{
	return m_indices;
}

void VertexBuffer::setColorsBuffer(const std::vector<glm::vec3>& colors)
{
	m_colors = colors;
} 

void VertexBuffer::setupVBO()
{
	if (!m_data_ready) return;



	glGenVertexArrays(1, &m_vao_id);
	glBindVertexArray(m_vao_id);


	if (!m_vertices.empty()) 
	{
		glGenBuffers(1, &m_vbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!m_normals.empty()) 
	{
		glGenBuffers(1, &m_nbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_nbo_id);
		glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	if (!m_texcoords.empty())
	{
		glGenBuffers(1, &m_tbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_tbo_id);
		glBufferData(GL_ARRAY_BUFFER, m_texcoords.size() * sizeof(glm::vec2), &m_texcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!m_colors.empty())
	{
		glGenBuffers(1, &m_cbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_cbo_id);
		glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(glm::vec3), &m_colors[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!m_indices.empty())
	{
		glGenBuffers(1, &m_ibo_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0],GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
	
  unsigned int errorCode = 0;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::cout << errorCode;
  }

	m_initialized = true;
}