//  ----------------------------------
//  Copyright © 2015, Brown University, Providence, RI.
//  
//  All Rights Reserved
//   
//  Use of the software is provided under the terms of the GNU General Public License version 3 
//  as published by the Free Software Foundation at http://www.gnu.org/licenses/gpl-3.0.html, provided 
//  that this copyright notice appear in all copies and that the name of Brown University not be used in 
//  advertising or publicity pertaining to the use or distribution of the software without specific written 
//  prior permission from Brown University.
//  
//  See license.txt for further information.
//  
//  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE WHICH IS 
//  PROVIDED “AS IS”, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
//  FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY 
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR FOR ANY DAMAGES WHATSOEVER RESULTING 
//  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
//  OTHER TORTIOUS ACTION, OR ANY OTHER LEGAL THEORY, ARISING OUT OF OR IN CONNECTION 
//  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
//  ----------------------------------
//  
///\file Shader.cpp
///\author Benjamin Knorlein
///\date 11/22/2017

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

#include "render/Shader.h"
#include <iostream>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif

Shader::Shader() : m_programID(0), m_shader(0), m_vertexShader(0), m_fragmentShader(0)
{
  /*m_shader = "Distortion";
  m_vertexShader = "varying vec2 texture_coordinate; \n"
      "void main()\n"
      "{\n"
      "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
      "	texture_coordinate = vec2(gl_MultiTexCoord0); \n"
      "}\n";
  m_fragmentShader = "varying vec2 texture_coordinate;\n"
      "uniform sampler2D displacement;\n"
      "uniform sampler2D texture;\n"
      "void main()\n"
      "{\n"
      "		vec4 disp_coords = texture2D(displacement,texture_coordinate);\n"
      "		gl_FragColor = texture2D(texture, disp_coords.xy);\n"
      "}\n";*/
}

Shader::~Shader()
{
  if (m_programID)
  {
    glDeleteProgram(m_programID);
    m_programID = 0;
  }
}

void Shader::bindProgram()
{
  if (!m_programID)
  {
    m_programID = compileShader();
  }
  glUseProgram(m_programID);
}

void Shader::unbindProgram()
{
  glUseProgram(0);
}

unsigned Shader::getProgram()
{
  if (!m_programID)
  {
    m_programID = compileShader();
  }

  return m_programID;
}

unsigned int Shader::compileShader()
{
  //std::cout << "compileShader shader 1" << std::endl;
  if (!m_shader || !m_vertexShader || !m_fragmentShader) {
    std::cerr << "could not create Shader" << std::endl;
    return 0;
  }

 //std::cout << "compileShader shader 1.5" << std::endl;
  GLuint programID = glCreateProgram();
 //std::cout << "compileShader shader 2" << std::endl;
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &m_vertexShader, NULL);
  glCompileShader(vertexShader);
 //std::cout << "compileShader shader 3" << std::endl;
  GLint shaderCompiled = GL_FALSE;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
  if (shaderCompiled != GL_TRUE)
  {
    std::cerr << "Error compiling vertex shader " << m_shader << std::endl;

    GLint maxLength = 0;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);
    std::string s(errorLog.begin(), errorLog.end());
    std::cerr << s.c_str() << std::endl;

    glDeleteProgram(programID);
    glDeleteShader(vertexShader);
    return 0;
  }
// std::cout << "compileShader shader 4" << std::endl;
  glAttachShader(programID, vertexShader);
  glDeleteShader(vertexShader); // the program hangs onto this once it's attached

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &m_fragmentShader, NULL);
  glCompileShader(fragmentShader);

 //std::cout << "compileShader shader 5" << std::endl;
  shaderCompiled = GL_FALSE;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
  if (shaderCompiled != GL_TRUE)
  {
    std::cerr << "Error compiling fragment shader " << m_shader << std::endl;

    GLint maxLength = 0;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);
    std::string s(errorLog.begin(), errorLog.end());
    std::cerr << s.c_str() << std::endl;

    glDeleteProgram(programID);
    glDeleteShader(fragmentShader);
    return 0;
  }
 //std::cout << "compileShader shader 6" << std::endl;
  glAttachShader(programID, fragmentShader);
  glDeleteShader(fragmentShader); // the program hangs onto this once it's attached

  glLinkProgram(programID);

  GLint success = GL_TRUE;
  glGetProgramiv(programID, GL_LINK_STATUS, &success);
  if (success != GL_TRUE)
  {
    std::cerr << "Error linking program " << m_shader << std::endl;
    glDeleteProgram(success);
    return 0;
  }

  glUseProgram(programID);
  glUseProgram(0);
// std::cout << "compileShader shader 7" << std::endl;
  return programID;
}
