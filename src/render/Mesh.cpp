#include "../include/render/Mesh.h"

Mesh::Mesh(Model* model, unsigned int volume_id):m_model(model),m_volume_id(0)
{

}
Mesh::~Mesh()
{
  delete m_model;
}

unsigned int Mesh::get_volume_id()  
{ 
  return m_volume_id; 
}

void Mesh::set_volume_id(unsigned int val) 
{
  m_volume_id = val; 
}

Model& Mesh::get_model()
{ 
  return *m_model; 
}

