//  ----------------------------------.

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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/VolumeRaycastShader.h"
#include "common/common.h"
#include "render/ShaderUniforms.h"

#include <glm/gtc/type_ptr.hpp>


VolumeRaycastShader::VolumeRaycastShader() : m_use_blending(false), m_blend_volume(0), m_blending_alpha(0), m_clip_min(0.0), m_clip_max(1.0)
{
	m_shader = "VolumeRaycastShader";
	m_vertexShader = "";
	m_fragmentShader = "";
}

VolumeRaycastShader::~VolumeRaycastShader()
{
}

void VolumeRaycastShader::render(glm::mat4& MVP, glm::mat4& clipPlane, glm::vec3& camPos)
{
	m_shader_program.start();
	if (m_use_blending)
	{
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_3D, m_blend_volume);
	}

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);

	std::string texture_uniform_name = m_texture_atlas_render ? "volume_2D" : "volume_3D";
	m_shader_program.setUniformi(texture_uniform_name.c_str(), 0);


	m_shader_program.setUniformi("lut", 1);
	m_shader_program.setUniformi("depth", 2);
	m_shader_program.setUniformi("blendVolume", 3);
	m_shader_program.setUniform("MVP", MVP);
	m_shader_program.setUniform("clipPlane", clipPlane);
	m_shader_program.setUniform("camPos", camPos);
	m_shader_program.setUniformf("threshold", m_threshold);
	m_shader_program.setUniformf("multiplier", m_multiplier);
	m_shader_program.setUniformi("clipping", m_clipping);
	m_shader_program.setUniformi("channel", m_channel);
	m_shader_program.setUniformi("useLut", m_useLut);
	m_shader_program.setUniformi("useMultiLut", m_useMultiLut);

	m_shader_program.setUniformf("slices", m_slices);
	m_shader_program.setUniformf("dimension", m_dim);
	m_shader_program.setUniformi("texture_atlas", m_texture_atlas_render);
	m_shader_program.setUniformi("useBlend", false);
	m_shader_program.setUniform("clip_min", m_clip_min);
	m_shader_program.setUniform("clip_max", m_clip_max);

	//////draw the triangles
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

	////unbind the shader
	m_shader_program.stop();
	glBindTexture(GL_TEXTURE_2D, 0);

	if (false)
	{
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_3D, 0);
	}
}

void VolumeRaycastShader::initGL(const std::string& shader_file_path)
{
	std::string m_ray_caster_vs_filepath = shader_file_path + OS_SLASH + std::string("raycast_shader.vert");
	std::string m_ray_caster_fs_filepath = shader_file_path + OS_SLASH + std::string("raycast_shader.frag");
	m_shader_program.LoadShaders(m_ray_caster_vs_filepath.c_str(), m_ray_caster_fs_filepath.c_str());
	m_shader_program.start();
	for (std::string uniform : ShaderUniforms::shader_uniforms)
	{
		m_shader_program.addUniform(uniform.c_str());
	}

	m_shader_program.stop();
}
