#include "Model.h"
#include <string>
#include <cassert>
#include "ShaderProgram.h"

Model::Model() : m_position(0), m_orientation(glm::quat()), m_scale(1), m_bounding_volumen_radius(1)
{
}

Model::~Model()
{
	delete m_Obj_Model;

	delete m_texture;
}

VertexBuffer &Model::objModel() const
{
	return *m_Obj_Model;
}

void Model::setObjModel(VertexBuffer *val)
{
	m_Obj_Model = val;
}

Texture &Model::texture(int index) const
{
	return *m_texture;
}

void Model::setTexture(Texture *val)
{
	m_texture = val;
}

void Model::render(ShaderProgram &shaderProgram)
{
	shaderProgram.setUniform("model_view_matrix", m_model_view_mtrx);

	assert(m_Obj_Model && "NO MODEL TO RENDER");
	// bind texture
	// Always load texture id 0. The model doesnt support multi texturing.
	m_texture->Bind();

	// render geometry
	if (m_Obj_Model)
	{
		m_Obj_Model->render();
	}

	// Unbind Text

	m_texture->UnBind();
}

glm::vec3 &Model::position()
{
	return m_position;
}

void Model::setPosition(glm::vec3 val)
{
	m_position = val;
}

glm::quat &Model::orientation()
{
	return m_orientation;
}

void Model::setOrientation(glm::quat val)
{
	m_orientation = val;
}

glm::vec3 &Model::scale()
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

bool Model::RayInstersection(const glm::vec3 &start, const glm::vec3 &rayDirection)
{

	glm::vec3 Position = m_position;

	float b = 2.f * (rayDirection.x * (start.x - Position.x) +
					 rayDirection.y * (start.y - Position.y) + rayDirection.z * (start.z - Position.z));
	float c = start.x * start.x - 2.f * start.x * Position.x + Position.x * Position.x + start.y * start.y - 2.f * start.y * Position.y + Position.y * Position.y + start.z * start.z - 2.f * start.z * Position.z + Position.z * Position.z - m_bounding_volumen_radius * m_bounding_volumen_radius;

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
