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
///\file TransferFunction.cpp
///\author Benjamin Knorlein
///\date 6/11/2019

#pragma once

#include "GL/glew.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "TransferFunction.h"
#include <glm/detail/type_vec4.hpp>
#include <ostream>
#include <iostream>

//transfer function (lookup table) colour values
const glm::vec4 jet_values[9] = { glm::vec4(0, 0, 0.5, 0),
glm::vec4(0, 0, 1, 0.1),
glm::vec4(0, 0.5, 1, 0.3),
glm::vec4(0, 1, 1, 0.5),
glm::vec4(0.5, 1, 0.5, 0.75),
glm::vec4(1, 1, 0, 0.8),
glm::vec4(1, 0.5, 0, 0.6),
glm::vec4(1, 0, 0, 0.5),
glm::vec4(0.5, 0, 0, 0.0) };

TransferFunction::TransferFunction() : m_texture_id{ -1 }, m_dataLength{ 0 }, m_data{ nullptr }
{
	computeJetFunction();
}

TransferFunction::TransferFunction(float* data, unsigned int dataLength) : m_texture_id{ -1 }, m_dataLength{ dataLength }
{
	m_data = new float[m_dataLength * 4];
	memcpy(m_data, data, sizeof(float)* m_dataLength * 4);
}

TransferFunction::~TransferFunction()
{
	if (texture_id() != 0)
		glDeleteTextures(1, &texture_id());
	if (m_data != nullptr)
		delete[]  m_data;
}

void TransferFunction::initGL()
{
	if (texture_id() != 0)
		glDeleteTextures(1, &texture_id());

	glGenTextures(1, &m_texture_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, m_texture_id);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, m_dataLength, 0, GL_RGBA, GL_FLOAT, m_data);
}

void TransferFunction::computeJetFunction()
{
	int indices[9];
	float pData[256][4];

	//fill the colour values at the place where the colour should be after interpolation
	for (int i = 0; i<9; i++) {
		int index = i * 28;
		pData[index][0] = jet_values[i].x;
		pData[index][1] = jet_values[i].y;
		pData[index][2] = jet_values[i].z;
		pData[index][3] = jet_values[i].w;
		indices[i] = index;
		std::cerr << indices[i] << std::endl;
	}

	//for each adjacent pair of colours, find the difference in the rgba values and then interpolate
	for (int j = 0; j<9 - 1; j++)
	{
		float dDataR = (pData[indices[j + 1]][0] - pData[indices[j]][0]);
		float dDataG = (pData[indices[j + 1]][1] - pData[indices[j]][1]);
		float dDataB = (pData[indices[j + 1]][2] - pData[indices[j]][2]);
		float dDataA = (pData[indices[j + 1]][3] - pData[indices[j]][3]);
		int dIndex = indices[j + 1] - indices[j];

		float dDataIncR = dDataR / float(dIndex);
		float dDataIncG = dDataG / float(dIndex);
		float dDataIncB = dDataB / float(dIndex);
		float dDataIncA = dDataA / float(dIndex);
		for (int i = indices[j] + 1; i<indices[j + 1]; i++)
		{
			pData[i][0] = (pData[i - 1][0] + dDataIncR);
			pData[i][1] = (pData[i - 1][1] + dDataIncG);
			pData[i][2] = (pData[i - 1][2] + dDataIncB);
			pData[i][3] = (pData[i - 1][3] + dDataIncA);
			m_dataLength++;
		}
	}
	m_data = new float[m_dataLength * 4];
	memcpy(m_data, pData, sizeof(float)* m_dataLength * 4);
}
