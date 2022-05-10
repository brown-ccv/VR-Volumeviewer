#include "ShaderProgram.h"
#include <fstream>
#include <vector>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


ShaderProgram::ShaderProgram() :m_progarm_id(0)
{
}

void ShaderProgram::LoadShaders(const std::string& vertex_filenname, const std::string& fragment_filename)
{
	int vertexShaderID = LoadShader(vertex_filenname.c_str(), GL_VERTEX_SHADER);
	int fragmentShaderID = LoadShader(fragment_filename.c_str(), GL_FRAGMENT_SHADER);


	int IsLinked;
	// Link the program
	printf("Linking program\n");

	m_progarm_id = glCreateProgram();

	glAttachShader(m_progarm_id, vertexShaderID);
	glAttachShader(m_progarm_id, fragmentShaderID);


	glBindAttribLocation(m_progarm_id, 0, "vertexPosition");
	glBindAttribLocation(m_progarm_id, 1, "texCoord");
	glBindAttribLocation(m_progarm_id, 2, "normals");

	glLinkProgram(m_progarm_id);

	glGetProgramiv(m_progarm_id, GL_LINK_STATUS, (int *)&IsLinked);
	if (IsLinked == false)
	{
		int maxLength;
		glGetProgramiv(m_progarm_id, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<char> VertexShaderErrorMessage(maxLength + 1);
		glGetProgramInfoLog(m_progarm_id, maxLength, &maxLength, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
		return;
	}

	glDetachShader(m_progarm_id, vertexShaderID);
	glDetachShader(m_progarm_id, fragmentShaderID);
	

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

}


ShaderProgram::~ShaderProgram()
{

}

void ShaderProgram::start()
{
	assert(m_progarm_id != 0 && "Shader program not initialized");
	glUseProgram(m_progarm_id);
}

void ShaderProgram::stop()
{
	glUseProgram(0);
}

void ShaderProgram::addUniform(const char * name)
{
	assert(m_progarm_id != 0 && "Shader program not initialized");
	int uniformLocation = glGetUniformLocation(m_progarm_id, name);
	if (uniformLocation != -1/*did not find the uniform*/)
	{
		std::string x(name);
		uniforms[x] = uniformLocation;
	}
	else
	{
		printf("uniform %s not linked in shader!\n", name);
	}
}

void ShaderProgram::setUniformi(const char * name, int value)
{
	glUniform1i(uniforms[name], value);
}

void ShaderProgram::setUniformf(const char * name, float value)
{
	glUniform1f(uniforms[name], value);
}

void ShaderProgram::setUniform(const char * name, glm::vec3& vector)
{
	glUniform3f(uniforms[name], vector.x, vector.y, vector.z);
}

void ShaderProgram::setUniform(const char * name, glm::vec4& vector)
{
	glUniform4f(uniforms[name], vector.x, vector.y, vector.z, vector.w);
}

void ShaderProgram::setUniform(const char * name, glm::mat4& matrix)
{
	if (!InUse())
	{
		start();
	}
	if (uniforms.find(name) == uniforms.end())
	{
		addUniform(name);
	}
	glUseProgram(m_progarm_id);
	glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::setUniformMatrix4fv(const char* name, const GLfloat* matrix)
{
	assert(m_progarm_id != 0 && "Shader program not initialized");
	if (uniforms.find(name) == uniforms.end())
	{
		addUniform(name);
	}
	glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, matrix);
}

GLuint ShaderProgram::GetProgramId()
{
	return m_progarm_id;
}

void ShaderProgram::VerifiProgram(GLuint programId)
{
	int Result = GL_FALSE;
	int InfoLogLength;
	// Check Vertex Shader
	glGetShaderiv(programId, GL_COMPILE_STATUS, &Result);

	if (Result == false) {
		glGetShaderiv(programId, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(programId, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	assert(Result);
}

bool ShaderProgram::InUse()
{
	GLint currentProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	return currentProgram == (GLint)m_progarm_id;
}

GLuint ShaderProgram::LoadShader(const char * filenname, int type)
{
	GLuint shaderId = glCreateShader(type);
	// Read the Vertex Shader code from the file
	std::string ShaderCode;
	std::ifstream ShaderStream(filenname, std::ios::in);

	if (ShaderStream.is_open()) {
		std::string Line = "";
		while (getline(ShaderStream, Line))
			ShaderCode += "\n" + Line;
		ShaderStream.close();
	}
	else {
		printf("Impossible to open %s !\n", filenname);
		getchar();
		return 0;
	}

	// Compile Shader
	printf("Compiling shader : %s\n", filenname);
	char const * SourcePointer = ShaderCode.c_str();
	glShaderSource(shaderId, 1, &SourcePointer, NULL);
	glCompileShader(shaderId);

	VerifiProgram(shaderId);
	return shaderId;
}
