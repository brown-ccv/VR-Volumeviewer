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
///\file VolumeRaycastShader.cpp
///\author Benjamin Knorlein
///\date 11/30/2017

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "VolumeRaycastShader.h"
#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

VolumeRaycastShader::VolumeRaycastShader() //: m_threshold{ 0.0f }, m_multiplier{ 0.5 }
{

	/*m_viewport[0] = 1.0;
	m_viewport[1] = 1.0;*/

	m_shader = "VolumeRaycastShader";

	m_vertexShader = "#version 330 core\n"
		"layout(location = 0) in vec3 vVertex;\n" //object space vertex position
		"uniform mat4 MVP; \n"   //combined modelview projection matrix
		"smooth out vec3 vUV; \n" //3D texture coordinates for texture lookup in the fragment shader
		"void main()\n"
		"{\n"
			//get the clipspace position 
			"gl_Position = MVP*vec4(vVertex, 1); \n"
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
		"uniform vec3 camPos;\n"						//camera position
		"uniform vec3 step_size;\n"					//ray step size
		"const int MAX_SAMPLES = 3000;\n"				//total samples for each ray march step
		"const vec3 texMin = vec3(0);\n"					//minimum texture access coordinate
		"const vec3 texMax = vec3(1);\n"					//maximum texture access coordinate
		"uniform int channel;\n"
		"uniform sampler1D lut;\n"					//transferfunction
		"uniform bool useLut;\n"
		"uniform sampler2D depth;\n"
		"uniform vec2 viewport;\n"
		"uniform mat4 P_inv; \n"
		"void main()\n"
		"{\n"
				//get the 3D texture coordinates for lookup into the volume dataset
				"vec3 dataPos = vUV; \n"
				//Getting the ray marching direction:
				//get the object space position by subracting 0.5 from the
				//3D texture coordinates. Then subtraact it from camera position
				//and normalize to get the ray marching direction
				"vec3 geomDir = normalize(camPos - (vUV-vec3(0.5f))); \n"
				//multiply the raymarching direction with the step size to get the
				//sub-step size we need to take at each raymarching step
				"vec3 dirStep = geomDir * step_size; \n"

				//Compute occlusion point in volume coordinates
				"float d = texture(depth, vec2(gl_FragCoord.x/viewport.x,gl_FragCoord.y/viewport.y)).r; \n"
				"vec4 d_ndc = vec4((gl_FragCoord.x / viewport.x - 0.5) * 2.0,(gl_FragCoord.y / viewport.y - 0.5) * 2.0, (d - 0.5) * 2.0, 1.0); \n"
				"d_ndc = P_inv * d_ndc; \n "
				"d_ndc = d_ndc / d_ndc.w; \n"

				//check which is closer to the camera dataPos or d_ndc
				"if(length(camPos - (vUV - vec3(0.5))) > length(d_ndc.xyz - (vUV - vec3(0.5)))) \n"
					"dataPos = d_ndc.xyz + vec3(0.5f); \n"

				//maximum iterations until camera position reached
				"float max_it = floor( length(camPos - (vUV-vec3(0.5))) / step_size.r); \n"

				//for all samples along the ray
				"for (int i = 0; i < MAX_SAMPLES; i++) { \n"

				// advance ray by dirstep
				"dataPos = dataPos + dirStep; \n"
				//break if behind camera
				"if (i > max_it) \n"
				"break; \n"

				//The two constants texMin and texMax have a value of vec3(-1,-1,-1)
				//and vec3(1,1,1) respectively. To determine if the data value is 
				//outside the volume data, we use the sign function. The sign function 
				//return -1 if the value is less than 0, 0 if the value is equal to 0 
				//and 1 if value is greater than 0. Hence, the sign function for the 
				//calculation (sign(dataPos-texMin) and sign (texMax-dataPos)) will 
				//give us vec3(1,1,1) at the possible minimum and maximum position. 
				//When we do a dot product between two vec3(1,1,1) we get the answer 3. 
				//So to be within the dataset limits, the dot product will return a 
				//value less than 3. If it is greater than 3, we are already out of 
				//the volume dataset
				"if (dot(sign(dataPos-texMin),sign(texMax-dataPos)) < 3.0) \n"
				"break; \n"

				//Stop when clipped
				"if(clipping){ \n"
				"vec4 p = clipPlane * vec4(dataPos, 1);\n"
				"if(p.y > 0.0f)\n"
				"break; \n"
				"}\n"

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
				"}"

				"if(useLut) \n"
				"sample = texture(lut, sample.a);"

				//assume alpha is the highest channel and gamma correction
				//"sample.a = pow(sample.a , multiplier); \n"  ///needs changing

				//threshold based on alpha
				"if (sample.a < threshold) continue;\n"

				//blending
				"vFragColor = vFragColor * (1.0 - sample.a) + sample * sample.a; \n"
				"} \n"
			//remove fragments for correct depthbuffer
			"if (vFragColor.a == 0.0f)"
				"discard;"
		"}\n";
}

VolumeRaycastShader::~VolumeRaycastShader()
{
}

void VolumeRaycastShader::render(glm::mat4 &MVP, glm::mat4 &clipPlane, glm::vec3 &camPos)
{
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
	glUniform2f(m_viewport_uniform, m_screen_size[0], m_screen_size[1]);
	glUniformMatrix4fv(m_P_inv_uniform, 1, GL_FALSE, glm::value_ptr(m_P_inv));

	//////draw the triangles
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
	////unbind the shader
	unbindProgram();

	glBindTexture(GL_TEXTURE_2D, 0);
}

void VolumeRaycastShader::initGL()
{
	bindProgram();
	//add attributes and uniforms
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
	m_viewport_uniform = glGetUniformLocation(m_programID, "viewport");
	m_depth_uniform = glGetUniformLocation(m_programID, "depth");
	m_P_inv_uniform = glGetUniformLocation(m_programID, "P_inv");

	////pass constant uniforms at initialization
	glUniform1i(m_volume_uniform, 0);
	glUniform1i(m_lut_uniform, 1);
	glUniform1i(m_depth_uniform, 2);

	unbindProgram();
}
