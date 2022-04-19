#ifndef MODEL_H
#define MODEL_H
#include "VertexBuffer.h"
#include "Texture.h"


class ShaderProgram;
class Model
{
public:
	
	~Model();

	VertexBuffer& objModel() const; 
	void setObjModel(VertexBuffer* val); 


	Texture& texture(int index) const; 
	void setTexture(Texture* val); 
	

	void render( ShaderProgram& shader);


	glm::vec3& position();
	void setPosition(glm::vec3 val);

	glm::quat& orientation(); 
	void setOrientation(glm::quat val); 

	glm::vec3& scale(); 
	void setScale(glm::vec3 val); 



	float boundingVolumenRadius() const; 
	void setBoundingVolumenRadius(float val); 

  bool RayInstersection(const glm::vec3& start,const  glm::vec3& rayDirection);

  bool isSelected();
  void seleted(bool );

	glm::mat4& MVMatrix()  { return m_model_view_mtrx; }
	void setMVMatrix(glm::mat4& val) { m_model_view_mtrx = val; }

	glm::mat4 modelViewM()&  { return m_model_view_mtrx; }

	void SetModelView(glm::mat4& val) { m_model_view_mtrx = val; }

private:

	Model();

	VertexBuffer* m_Obj_Model;
	Texture* m_texture;

	friend class GLMLoader;
	friend class AssimpLoader;
	friend class Terrain;

	glm::vec3 m_position;
	glm::quat m_orientation;
	glm::vec3 m_scale;

	glm::mat4 m_model_view_mtrx;

	float m_bounding_volumen_radius;

  bool m_selected;

	std::vector<glm::mat4> m_model_view_matrix;
};

#endif
