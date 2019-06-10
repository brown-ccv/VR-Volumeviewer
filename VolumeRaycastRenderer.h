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

#pragma once

#ifndef VOLUMESLICERCASTRENDER_H
#define VOLUMESLICERCASTRENDER_H

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

#include <glm/glm.hpp>
#include "VolumeRaycastShader.h"
#include "VolumeRenderer.h"

class VolumeRaycastRenderer : public  VolumeRenderer
	{
	public:
		VolumeRaycastRenderer();
		~VolumeRaycastRenderer();

		virtual void initGL() override;
		virtual void render(Volume* volume, const glm::mat4 &MV, glm::mat4 &P, float z_scale) override;

		virtual void set_threshold(float threshold) override;
		virtual void set_multiplier(float multiplier) override;

	private:
		////function to get the max (abs) dimension of the given vertex v
		//int FindAbsMax(glm::vec3 v);
		//
		////main slicing function
		//void SliceVolume();

		////unit cube vertices
		//static const glm::vec3 vertexList[8];

		//////unit cube edges
		//static const int edgeList[8][12];
		//static const int edges[12][2];

		////sliced vertices
		//static const int MAX_SLICES = 256;
		//glm::vec3 vTextureSlices[MAX_SLICES * 12];

		////total number of slices current used
		//int num_slices = 1024;

		////current viewing direction
		//glm::vec3 viewDir;

		////volume vertex array and buffer objects
		GLuint cubeVBOID;
		GLuint cubeVAOID;
		GLuint cubeIndicesID;

		//3D texture slicing shader
		VolumeRaycastShader shader;
	};
#endif // VOLUMESLICERCASTRENDER_H
