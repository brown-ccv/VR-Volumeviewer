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
///\file VolumeSliceRcastRender.cpp
///\author Benjamin Knorlein
///\date 05/24/2019
/// Based on the book : OpenGL Development Cookbook  by Muhammad Mobeen Movania

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "VolumeRaycastRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

////for floating point inaccuracy
//const float EPSILON = 0.0001f;

VolumeRaycastRenderer::VolumeRaycastRenderer() //: num_slices{ MAX_SLICES }
{

}

VolumeRaycastRenderer::~VolumeRaycastRenderer()
{
	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);
}

void VolumeRaycastRenderer::initGL()
{
	////Load and init the texture slicing shader
	shader.initGL();

	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVBOID);
	glGenBuffers(1, &cubeIndicesID);

	//unit cube vertices 
	glm::vec3 vertices[8] = { glm::vec3(-0.5f, -0.5f, -0.5f),//0
		glm::vec3(0.5f, -0.5f, -0.5f), //1
		glm::vec3(0.5f, 0.5f, -0.5f), // 2
		glm::vec3(-0.5f, 0.5f, -0.5f), //3
		glm::vec3(-0.5f, -0.5f, 0.5f), //4
		glm::vec3(0.5f, -0.5f, 0.5f), //5
		glm::vec3(0.5f, 0.5f, 0.5f), //6
		glm::vec3(-0.5f, 0.5f, 0.5f) }; //7

	//unit cube indices
	GLushort cubeIndices[36] = { 0, 5, 4,
		5, 0, 1,
		3, 7, 6,
		3, 6, 2,
		7, 4, 6,
		6, 4, 5,
		2, 1, 3,
		3, 1, 0,
		3, 0, 7,
		7, 0, 4,
		6, 5, 2,
		2, 5, 1 };
	glBindVertexArray(cubeVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBOID);
	//pass cube vertices to buffer object memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

	//enable vertex attributre array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//pass indices to element array  buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void VolumeRaycastRenderer::render(Volume* volume, const glm::mat4 &MV, glm::mat4 &P, float z_scale)
{
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, volume->get_texture_id());

	////enable alpha blending (use over operator)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	////now do the z_scaling
	glm::mat4 MV_tmp = glm::scale(MV, glm::vec3(1, 1, z_scale));

	////get the combined modelview projection matrix
	glm::mat4 MVP = P*MV_tmp;
	glm::mat4 clipPlane;
	if (m_clipping){
		clipPlane = glm::translate(m_clipPlane * MV_tmp, glm::vec3(-0.5f));
		shader.set_clipping(true);
	} else
	{
		shader.set_clipping(false);
	}
	glm::vec3 camPos = glm::vec4(glm::inverse(MV_tmp)*glm::vec4(0, 0, 0, 1));
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBindVertexArray(cubeVAOID);
	////use the volume shader
	shader.set_stepSize(1.0f / 256.0f, 1.0f / 256.0f, 1.0f / 256.0f);
	shader.render(MVP, clipPlane, camPos);
	glDisable(GL_CULL_FACE);

	////disable blending
	glBindVertexArray(0);
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDepthMask(GL_TRUE);
}

void VolumeRaycastRenderer::set_threshold(float threshold)
{
	shader.set_threshold(threshold);
}

void VolumeRaycastRenderer::set_multiplier(float multiplier)
{
	shader.set_multiplier(multiplier);
}
