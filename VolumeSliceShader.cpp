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
///\file VolumeSliceShader.cpp
///\author Benjamin Knorlein
///\date 11/30/2017

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "VolumeSliceShader.h"
#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

VolumeSliceShader::VolumeSliceShader() : m_threshold{ 0.0f }, m_multiplier{ 0.5 }
{
	m_shader = "VolumeSliceRender";

	m_vertexShader = "#version 330 core\n"
		"layout(location = 0) in vec3 vVertex; \n" //object space vertex position
		"uniform mat4 MVP; \n"   //combined modelview projection matrix
		"smooth out vec3 vUV; \n" //3D texture coordinates for texture lookup in the fragment shader
		"void main()\n"
		"{\n"
			//get the clipspace position 
			"gl_Position = MVP*vec4(vVertex.xyz, 1); \n"
			//get the 3D texture coordinates by adding (0.5,0.5,0.5) to the object space 
			//vertex position. Since the unit cube is at origin (min: (-0.5,-0.5,-0.5) and max: (0.5,0.5,0.5))
			//adding (0.5,0.5,0.5) to the unit cube object space position gives us values from (0,0,0) to 
			//(1,1,1)
			"vUV = vVertex + vec3(0.5); \n"
		"}\n";

	m_fragmentShader = "#version 330 core\n"
		"layout(location = 0) out vec4 vFragColor; \n"	//fragment shader output
		"smooth in vec3 vUV; \n"						//3D texture coordinates form vertex shader interpolated by rasterizer
		"uniform sampler3D volume;\n"					//volume dataset
		"uniform mat4 clipPlane; \n"
		"uniform bool clipping;\n"
		"uniform float threshold;\n"
		"uniform float multiplier;\n"
		"void main()\n"
		"{\n"

		"if(clipping){ \n"
			"vec4 p = clipPlane * vec4(vUV, 1);\n"
			"if(p.y > 0.0f)\n"
			"discard; \n"
		"}\n"

		//Here. we sample the volume dataset using the 3D texture coordinates from the vertex shader.
		"vec4 c_out = texture(volume, vUV) ; \n"
		"c_out.a = pow(max(c_out.r, max(c_out.g,c_out.b)),multiplier) ; "
		"c_out.a = (c_out.a > threshold) ? c_out.a : 0.0f ;\n"

		//"c_out.rgb = c_out.rgb * multiplier;\n"
		
		//remove fragments for correct depthbuffer
		"if (c_out.a == 0.0f)"
			"discard;"
		"vFragColor = c_out;\n"
		"}\n";
}

VolumeSliceShader::~VolumeSliceShader()
{
}

void VolumeSliceShader::render(glm::mat4& MVP, glm::mat4 &clipPlane, GLsizei count)
{
	bindProgram();
	////pass the shader uniform
	glUniformMatrix4fv(m_MVP_uniform, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniformMatrix4fv(m_clipPlane_uniform, 1, GL_FALSE, glm::value_ptr(clipPlane));
	glUniform1f(m_threshold_uniform, m_threshold);
	glUniform1f(m_multiplier_uniform, m_multiplier);
	glUniform1i(m_clipping_uniform, m_clipping);

	////draw the triangles
	glDrawArrays(GL_TRIANGLES, 0, count);
	////unbind the shader
	unbindProgram();
}

void VolumeSliceShader::initGL()
{
	bindProgram();
	//add attributes and uniforms
	m_volume_uniform = glGetUniformLocation(m_programID, "volume");	
	m_MVP_uniform = glGetUniformLocation(m_programID, "MVP");
	m_vVertex_attribute = glGetAttribLocation(m_programID, "vVertex");
	m_threshold_uniform = glGetUniformLocation(m_programID, "threshold");
	m_multiplier_uniform = glGetUniformLocation(m_programID, "multiplier");
	m_viewport_uniform = glGetUniformLocation(m_programID, "viewport");
	m_clipping_uniform = glGetUniformLocation(m_programID, "clipping");
	m_clipPlane_uniform = glGetUniformLocation(m_programID, "clipPlane");

	//pass constant uniforms at initialization
	glUniform1i(m_volume_uniform, 0);
	unbindProgram();
}
