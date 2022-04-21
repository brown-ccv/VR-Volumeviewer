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
///\file Labels.h
///\author Benjamin Knorlein
///\date 6/25/2019

#ifndef LABELS_H
#define LABELS_H
#include <vector>
#include <glm/mat4x4.hpp>
#include <string>
#include <map>

class Model;
class Texture;
class VRVolumeApp;
class ShaderProgram;

struct LabelBillboard
{
  unsigned int line_vba;
  Texture *label_texture;
  Model *label_model;
  glm::vec3 position;
};

class Labels
{
public:
  Labels(ShaderProgram &lines_shader, ShaderProgram &plane_shader);
  ~Labels();

  void add(std::string texture, float x, float y, float z, float textPosZ, float size, int volume);
  void drawLabels(glm::mat4 MV, glm::mat4 projection_matrix, glm::mat4 &headpose, float z_scale);
  void drawLines();

  void clear();
  void set_parent_directory(std::string &directory);

private:
  unsigned int create_line_vba(glm::vec3 &start, glm::vec3 &end);

  std::vector<LabelBillboard> m_billboard_labels;

  std::vector<std::string> m_text;
  std::vector<glm::vec3> m_position;
  std::vector<glm::vec3> m_position2;
  std::vector<int> m_size;
  std::vector<int> m_volume;

  std::vector<unsigned int> m_lines_vba;
  std::vector<unsigned int> m_lines_vbo;
  bool m_init_plane_model;
  Model *m_plane_model;

  std::map<std::string, Texture *> m_texture_cache;
  std::string m_parent_directory;

  ShaderProgram &m_lines_shader_program;
  ShaderProgram &m_plane_shader_program;
};

#endif // LABELS_H
