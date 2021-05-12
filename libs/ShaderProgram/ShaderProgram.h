#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_map>




class ShaderProgram
{

public:
	ShaderProgram();
	void LoadShaders(const std::string& vertex_filenname, const std::string& fragment_filename);
	~ShaderProgram();
	void start();
	void stop();

	void addUniform(const char* name);

	void setUniformi(const char* name, int value);
	void setUniformf(const char* name, float value);
	void setUniform(const char* name, glm::vec3& vector);
	void setUniform(const char* name, glm::vec4& vector);
	void setUniform(const char* name, glm::mat4& matrix);
	void setUniformMatrix4fv(const char* name, const GLfloat* matrix);

	GLuint GetProgramId();

private:

	GLuint m_progarm_id;
	void VerifiProgram(GLuint programId);
	bool InUse();
	GLuint LoadShader(const char* vertex_filenname, int type);
	std::unordered_map<std::string, GLint> uniforms;
};

#endif