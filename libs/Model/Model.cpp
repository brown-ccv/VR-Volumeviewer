#include "Model.h"
#include <string>
#include <cassert>


Model::Model():m_position(0),m_orientation(glm::quat()),m_scale(1), m_bounding_volumen_radius(1)
{

}


Model::~Model()
{
	delete m_Obj_Model;
	for (int i = 0 ; i < m_textures.size(); ++i)
	{
		delete m_textures[i];
	}
		
}

VertexBuffer& Model::objModel() const
{
	return *m_Obj_Model;
}

void Model::setObjModel(VertexBuffer* val)
{
	m_Obj_Model = val;
}

Texture& Model::texture(int index) const
{
	return *m_textures[index];
}

void Model::addTexture(Texture* val)
{
	m_textures.push_back(val);
}

void Model::render(ShaderProgram& shaderProgram)
{

	/*glm::mat4 m_model = glm::mat4(1.0f);
	m_model *= glm::translate(myPosition);
	m_model *= glm::toMat4(myOrientation);
	m_model *= glm::scale(myScale);*/

	//shaderProgram.setUniform("m", m_model);
	shaderProgram.setUniform("mv", m_model_view_mtrx);

	assert(m_Obj_Model && "NO MODEL TO RENDER");
	// bind texture
	for (int i = 0; i < m_textures.size(); ++i)
	{
		m_textures[i]->Bind(i);
	}

	//render geometry
	if (m_Obj_Model)
	{
		m_Obj_Model->render();
	}
}

glm::vec3& Model::position() 
{
	return m_position;
}

void Model::setPosition(glm::vec3 val)
{
	m_position = val;
}

glm::quat& Model::orientation() 
{
	return m_orientation;
}

void Model::setOrientation(glm::quat val)
{
	m_orientation = val;
}

glm::vec3& Model::scale() 
{
	return m_scale;
}

void Model::setScale(glm::vec3 val)
{
	m_scale = val;
}

float Model::boundingVolumenRadius() const
{
	return m_bounding_volumen_radius;
}

void Model::setBoundingVolumenRadius(float val)
{
	m_bounding_volumen_radius = val;
}

bool Model::RayInstersection(const glm::vec3& start, const glm::vec3& rayDirection)
{
  
  glm::vec3 Position = m_position;

  float b = 2.f * (rayDirection.x * (start.x - Position.x) +
    rayDirection.y * (start.y - Position.y) + rayDirection.z * (start.z - Position.z));
  float c = start.x * start.x - 2.f * start.x * Position.x + Position.x * Position.x
    + start.y * start.y - 2.f * start.y * Position.y + Position.y * Position.y
    + start.z * start.z - 2.f * start.z * Position.z + Position.z * Position.z - m_bounding_volumen_radius * m_bounding_volumen_radius;

  float discr = (b * b - 4.f * c);
  if (discr < 0.f)
  {
    m_selected = false;
    
  }
  else
  {
    m_selected = true;
    
  }

  return m_selected;
}

bool Model::isSelected()
{
  return m_selected;
}

void Model::seleted(bool selected)
{
  m_selected = selected;
}
