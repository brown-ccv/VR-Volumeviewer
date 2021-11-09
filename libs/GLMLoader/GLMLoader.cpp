#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

#define  _CRT_SECURE_NO_WARNINGS

#include "GLMLoader.h"
#include <iostream>
#include <cassert>
#include <math.h>

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

#ifndef ANGLE_THRES
#define ANGLE_THRES 0
#endif

#ifndef M_PI
#define M_PI 3.14159265
#endif

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "GLMLoader.h"
#include "VertexBuffer.h"
#include "Model.h"


/* GLMtriangle: Structure that defines a triangle in a model.
*/
typedef struct _GLMtriangle {
  GLuint vindices[3];			/* array of triangle vertex indices */
  GLuint nindices[3];			/* array of triangle normal indices */
  GLuint txindices[3];			/* array of triangle vertex indices */
  GLuint findex;			/* index of triangle facet normal */
} GLMtriangle;

/* _GLMnode: general purpose node
*/
typedef struct _GLMnode {
  GLuint           index;
  GLboolean        averaged;
  struct _GLMnode* next;
} GLMnode;

/* glmCross: compute the cross product of two vectors
*
* u - array of 3 GLfloats (GLfloat u[3])
* v - array of 3 GLfloats (GLfloat v[3])
* n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
*/
static GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
  assert(u); assert(v); assert(n);

  n[0] = u[1] * v[2] - u[2] * v[1];
  n[1] = u[2] * v[0] - u[0] * v[2];
  n[2] = u[0] * v[1] - u[1] * v[0];
}

/* glmNormalize: normalize a vector
*
* v - array of 3 GLfloats (GLfloat v[3]) to be normalized
*/
static GLvoid
glmNormalize(GLfloat* v)
{
  GLfloat l;

  assert(v);

  l = (GLfloat)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] /= l;
  v[1] /= l;
  v[2] /= l;
}

/* glmDot: compute the dot product of two vectors
*
* u - array of 3 GLfloats (GLfloat u[3])
* v - array of 3 GLfloats (GLfloat v[3])
*/
static GLfloat
glmDot(GLfloat* u, GLfloat* v)
{
  assert(u); assert(v);

  return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}


Model* GLMLoader::loadObjModel(std::string& filename)
{
  tinyobj::ObjReaderConfig reader_config;
  tinyobj::ObjReader reader;
  if (!reader.ParseFromFile(filename, reader_config)) {
    if (!reader.Error().empty()) {
      std::cerr << "TinyObjReader: " << reader.Error();
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  std::vector<float> vertices2 = attrib.GetVertices();
  //std::vector<float> normals = attrib.normals;
  std::vector<float> text = attrib.texcoords;


  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> textcoords;
  std::vector<glm::vec3> normals;
  std::vector<unsigned int> indices;
  std::unordered_map<glm::vec3, int> uniqueVertices;
  std::unordered_map<glm::vec2, int> uniqueTextCoords;
  std::unordered_map<glm::vec3, int> uniqueNormals;

  for (const auto& shape : shapes)
  {
    for (const auto& index : shape.mesh.indices)
    {
      glm::vec3 position{
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2] };

      glm::vec2 text_coord{
       attrib.texcoords[2 * index.texcoord_index + 0],
       attrib.texcoords[2 * index.texcoord_index + 1] };

      // Load normals per vertex
      /*
      glm::vec3 normal{
      attrib.texcoords[3 * index.normal_index + 0],
      attrib.texcoords[3 * index.normal_index + 1],
      attrib.texcoords[3 * index.normal_index + 2] };*/

      if (uniqueVertices.count(position) == 0)
      {
        uniqueVertices[position] = static_cast<int>(vertices.size());
        vertices.push_back(position);
      }

      if (uniqueTextCoords.count(text_coord) == 0)
      {
        uniqueTextCoords[text_coord] = static_cast<int>(textcoords.size());
        textcoords.push_back(text_coord);
      }

      // Load normals per vertex
      /*
      if (uniqueNormals.count(normal) == 0)
      {
        uniqueNormals[normal] = static_cast<int>(normals.size());
        normals.push_back(normal);
      }*/

      indices.push_back(uniqueVertices[position]);
    }
  }

  std::cout << "num vertices2 " << vertices2.size() << std::endl;
  std::cout << "num vertices " << vertices.size() << std::endl;
  std::cout << "num normals " << normals.size() << std::endl;
  std::cout << "num text " << text.size() << std::endl;
  std::cout << "num text2 " << textcoords.size() << std::endl;


  VertexBuffer* vertexObject = new VertexBuffer();

  vertexObject->setData(vertices, normals, textcoords, indices);


  // return model
  Model* model = new Model();
  model->setObjModel(vertexObject);

  return model;


}

