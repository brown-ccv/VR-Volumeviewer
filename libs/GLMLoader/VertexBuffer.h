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
///\file VertexBuffer.h
///\author Benjamin Knorlein
///\date 07/29/2016
///\modified  Camilo Diaz
///\date 09/15/2018

#ifndef VERTEXBUFFER_H_
#define VERTEXBUFFER_H_
#include <mutex>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

class VertexBuffer
{
public:
	VertexBuffer();

	~VertexBuffer();

	
	void setData(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec2>& texcoords, const std::vector<unsigned int>& indices);

	void render();


	std::vector<glm::vec3>& vertices();

	std::vector<glm::vec3>& normals();

	std::vector<glm::vec2>& texcoords();

	std::vector<unsigned int>& indices(); 

	void setColorsBuffer(const std::vector<glm::vec3>& colors);


private:
	bool m_initialized;

	void setupVBO();
	unsigned int m_vao_id;
	unsigned int m_vbo_id;
	unsigned int m_nbo_id;
	unsigned int m_tbo_id;
	unsigned int m_cbo_id;
	unsigned int m_ibo_id;
	unsigned int m_numvertices;
	unsigned int m_num_indices;

	bool m_data_ready;

	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texcoords;
	std::vector<glm::vec3> m_colors;
	std::vector<unsigned int> m_indices;

	std::mutex mutex;

};


#endif // FRAMEBUFFER_H_