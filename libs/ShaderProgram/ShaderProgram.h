#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL


#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

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
	void setUniform(const char* name, glm::vec2& vector);
	void setUniform(const char* name, glm::ivec2& vector);
	void setUniform(const char* name, glm::ivec3& vector);
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