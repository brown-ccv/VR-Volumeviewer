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



VertexBuffer::VertexBuffer() : myInitialised(false), myVaoId(0), myVboId(0), myNboId(0), myTboId(0),
myIboId(), myDataReady(false), myNumIndices(0)
{

}

VertexBuffer::~VertexBuffer()
{
	if (myInitialised)
	{
		if (myVboId != 0)glDeleteBuffers(1, &myVboId);
		if (myNboId != 0)glDeleteBuffers(1, &myNboId);
		if (myTboId != 0)glDeleteBuffers(1, &myTboId);
		if (myIboId != 0)glDeleteBuffers(1, &myIboId);

		myInitialised = false;
	}
	
}

void VertexBuffer::setData(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
	std::vector<glm::vec2>& texcoords, std::vector<unsigned int>& indices)
{

	if (vertices.size()) {
		myVertices = vertices;
	}

	if (normals.size()) {
		myNormals = normals;
	}

	if (texcoords.size()) {
		myTexcoords = texcoords;
	}

	if (indices.size()) {
		myNumIndices = indices.size();
		myIndices = indices;
	}
	myDataReady = true;
}

void VertexBuffer::render()
{
	std::lock_guard<std::mutex> guard(mutex);
	if (!myInitialised)
	{
		if (!myDataReady)
			return;

		setupVBO();
	}

	glBindVertexArray(myVaoId);
	glDrawElements(GL_TRIANGLES, myNumIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);



}

std::vector<glm::vec3>& VertexBuffer::vertices()
{
	return myVertices;
}

std::vector<glm::vec3>& VertexBuffer::normals()
{
	return myNormals;
}


std::vector<glm::vec2>& VertexBuffer::texcoords()
{
	return myTexcoords;
}


std::vector<unsigned int>& VertexBuffer::indices()
{
	return myIndices;
}

void VertexBuffer::setColorsBuffer(const std::vector<glm::vec3>& colors)
{
	myColors = colors;
} 

void VertexBuffer::setupVBO()
{
	if (!myDataReady) return;



	glGenVertexArrays(1, &myVaoId);
	glBindVertexArray(myVaoId);


	if (!myVertices.empty()) 
	{
		glGenBuffers(1, &myVboId);
		glBindBuffer(GL_ARRAY_BUFFER, myVboId);
		glBufferData(GL_ARRAY_BUFFER, myVertices.size() * sizeof(glm::vec3), &myVertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!myNormals.empty()) 
	{
		glGenBuffers(1, &myNboId);
		glBindBuffer(GL_ARRAY_BUFFER, myNboId);
		glBufferData(GL_ARRAY_BUFFER, myNormals.size() * sizeof(glm::vec3), &myNormals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	if (!myTexcoords.empty())
	{
		glGenBuffers(1, &myTboId);
		glBindBuffer(GL_ARRAY_BUFFER, myTboId);
		glBufferData(GL_ARRAY_BUFFER, myTexcoords.size() * sizeof(glm::vec2), &myTexcoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!myColors.empty())
	{
		glGenBuffers(1, &myCboId);
		glBindBuffer(GL_ARRAY_BUFFER, myCboId);
		glBufferData(GL_ARRAY_BUFFER, myColors.size() * sizeof(glm::vec3), &myColors[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!myIndices.empty())
	{
		glGenBuffers(1, &myIboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myIboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, myIndices.size() * sizeof(unsigned int), &myIndices[0],GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
	
  unsigned int errorCode = 0;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::cout << errorCode;
  }

	myInitialised = true;
}