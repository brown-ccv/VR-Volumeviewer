

#ifndef MESH_H
#define MESH_H
#include "Model.h"

class Mesh
{
  
public:

  Mesh(Model*, unsigned int volume_id);
  virtual ~Mesh();

  unsigned int get_volume_id(); 

  void set_volume_id(unsigned int val); 

  Model& get_model(); 
  
private:
  Model* m_model;
  unsigned int m_volume_id;
};


#endif 

