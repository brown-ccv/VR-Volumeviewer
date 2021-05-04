//#ifndef MODEL_H
//#define MODEL_H
//#include "../GLMLoader/VertexBuffer.h"
//#include "../Texture/Texture.h"
//#include "../ShaderProgram/ShaderProgram.h"
//
//class Model
//{
//public:
//	
//	~Model();
//
//	VertexBuffer& objModel() const; 
//	void setObjModel(VertexBuffer* val); 
//
//
//	Texture& texture(int index) const; 
//	void addTexture(Texture* val); 
//	
//
//	void render(ShaderProgram shader);
//
//
//	glm::vec3& position();
//	void setPosition(glm::vec3 val);
//
//	glm::quat& orientation(); 
//	void setOrientation(glm::quat val); 
//
//	glm::vec3& scale(); 
//	void setScale(glm::vec3 val); 
//
//
//	float boundingVolumenRadius() const; 
//	void setBoundingVolumenRadius(float val); 
//
//  bool RayInstersection(vec3 start, vec3 rayDirection);
//
//  bool isSelected();
//  void seleted(bool );
//
//private:
//
//	Model();
//
//	VertexBuffer* myObjModel;
//	std::vector<Texture*> myTextures;
//
//	friend class GLMLoader;
//	friend class AssimpLoader;
//	friend class Terrain;
//
//	glm::vec3 myPosition;
//	glm::quat myOrientation;
//	glm::vec3 myScale;
//
//	float myBoundingVolumenRadius;
//
//  bool mySelected;
//};
//
//#endif
