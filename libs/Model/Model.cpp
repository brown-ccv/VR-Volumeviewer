#include "../Model/Model.h"
#include "../GLMLoader/GLMLoader.h"
#include <string>
#include <cassert>


Model::Model():myPosition(0),myOrientation(quat()),myScale(1), myBoundingVolumenRadius(1)
{

}


Model::~Model()
{
	delete myObjModel;
	for (int i = 0 ; i < myTextures.size(); ++i)
	{
		delete myTextures[i];
	}
		
}

VertexBuffer& Model::objModel() const
{
	return *myObjModel;
}

void Model::setObjModel(VertexBuffer* val)
{
	myObjModel = val;
}

Texture& Model::texture(int index) const
{
	return *myTextures[index];
}

void Model::addTexture(Texture* val)
{
	myTextures.push_back(val);
}

void Model::render(ShaderProgram shaderProgram)
{

	/*glm::mat4 m_model = glm::mat4(1.0f);
	m_model *= glm::translate(myPosition);
	m_model *= glm::toMat4(myOrientation);
	m_model *= glm::scale(myScale);*/

	//shaderProgram.setUniform("m", m_model);
	shaderProgram.setUniform("mv", MV);

	assert(myObjModel && "NO MODEL TO RENDER");
	// bind texture
	for (int i = 0; i < myTextures.size(); ++i)
	{
		myTextures[i]->Bind(i);
	}

	//render geometry
	if (myObjModel)
	{
		myObjModel->render();
	}
}

glm::vec3& Model::position() 
{
	return myPosition;
}

void Model::setPosition(glm::vec3 val)
{
	myPosition = val;
}

glm::quat& Model::orientation() 
{
	return myOrientation;
}

void Model::setOrientation(glm::quat val)
{
	myOrientation = val;
}

glm::vec3& Model::scale() 
{
	return myScale;
}

void Model::setScale(glm::vec3 val)
{
	myScale = val;
}

float Model::boundingVolumenRadius() const
{
	return myBoundingVolumenRadius;
}

void Model::setBoundingVolumenRadius(float val)
{
	myBoundingVolumenRadius = val;
}

bool Model::RayInstersection(vec3 start, vec3 rayDirection)
{
  
  glm::vec3 Position = myPosition;

  float b = 2 * (rayDirection.x * (start.x - Position.x) +
    rayDirection.y * (start.y - Position.y) + rayDirection.z * (start.z - Position.z));
  float c = start.x * start.x - 2 * start.x * Position.x + Position.x * Position.x
    + start.y * start.y - 2 * start.y * Position.y + Position.y * Position.y
    + start.z * start.z - 2 * start.z * Position.z + Position.z * Position.z - myBoundingVolumenRadius * myBoundingVolumenRadius;

  float discr = (b * b - 4 * c);
  if (discr < 0)
  {
    mySelected = false;
    
  }
  else
  {
    mySelected = true;
    
  }

  return mySelected;
}

bool Model::isSelected()
{
  return mySelected;
}

void Model::seleted(bool selected)
{
  mySelected = selected;
}
