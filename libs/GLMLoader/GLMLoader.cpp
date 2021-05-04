#include <GL/glew.h>

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

#include "../GLMLoader/GLMLoader.h"
//#include "../glm.h"


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


Model* GLMLoader::loadModel(std::string& filename)
{
	FILE*     file;

	/* open the file */
	file = fopen(filename.c_str(), "r");
	if (!file)
		return NULL;

	unsigned int m_numvertices;
	float* m_vertices;
	float* m_normals;
	float* m_textures;

	

	unsigned int * m_indices;

	GLuint    numvertices;		/* number of vertices in model */
	GLuint    numnormals;		/* number of normals in model */
	GLuint    numsUV;		/* number of normals in model */
	GLuint    numtriangles;		/* number of triangles in model */
	char      buf[128];
	unsigned  v, n, t;

	numvertices = numnormals = numsUV = numtriangles = 0;
	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
								/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1]) {
			case '\0':			/* vertex */
								/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numvertices++;
				break;
			case 'n':				/* normal */
									/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numnormals++;
				break;
			case 't':				/* texture */
									/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				numsUV++;
				break;
			default:
				fgets(buf, sizeof(buf), file);
				break;
			}
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				fscanf(file, "%d//%d", &v, &n);
				numtriangles++;
				while (fscanf(file, "%d//%d", &v, &n) > 0) {
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				numtriangles++;
				while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				fscanf(file, "%d/%d", &v, &t);
				fscanf(file, "%d/%d", &v, &t);
				numtriangles++;
				while (fscanf(file, "%d/%d", &v, &t) > 0) {
					numtriangles++;
				}
			}
			else {
				/* v */
				fscanf(file, "%d", &v);
				fscanf(file, "%d", &v);
				numtriangles++;
				while (fscanf(file, "%d", &v) > 0) {
					numtriangles++;
				}
			}
			break;
		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}

	/* allocate memory */
	GLfloat* vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * numvertices);
	GLMtriangle* triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) * numtriangles);
	GLfloat* normals = 0;
	GLfloat* texcoords = 0;

	if (numnormals > 0) {
		normals = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (numnormals));
	}

	if (numsUV)
	{
		texcoords = (GLfloat*)malloc(sizeof(GLfloat) * 2 * (numsUV));
	}

	rewind(file);

	numtriangles = numvertices = numnormals = numsUV = 0;

	while (fscanf(file, "%s", buf) != EOF) {
		switch (buf[0]) {
		case '#':				/* comment */
								/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1]) {
			case '\0':			/* vertex */
				fscanf(file, "%f %f %f",
					&vertices[3 * numvertices + 0],
					&vertices[3 * numvertices + 1],
					&vertices[3 * numvertices + 2]);
				numvertices++;
				break;
			case 'n':				/* normal */
				fscanf(file, "%f %f %f",
					&normals[3 * numnormals + 0],
					&normals[3 * numnormals + 1],
					&normals[3 * numnormals + 2]);
				numnormals++;
				break;
			case 't':           /* texcoord */
				fscanf(file, "%f %f",
					&texcoords[2 * numsUV + 0],
					&texcoords[2 * numsUV + 1]);
				numsUV++;
				break;
			default:
				fgets(buf, sizeof(buf), file);
				break;
			}
			break;
		case 'f':				/* face */
			v = n = t = 0;
			fscanf(file, "%s", buf);
			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//")) {
				/* v//n */
				sscanf(buf, "%d//%d", &v, &n);
				triangles[numtriangles].vindices[0] = v;
				triangles[numtriangles].nindices[0] = n;
				fscanf(file, "%d//%d", &v, &n);
				triangles[numtriangles].vindices[1] = v;
				triangles[numtriangles].nindices[1] = n;
				fscanf(file, "%d//%d", &v, &n);
				triangles[numtriangles].vindices[2] = v;
				triangles[numtriangles].nindices[2] = n;
				numtriangles++;
				while (fscanf(file, "%d//%d", &v, &n) > 0) {
					triangles[numtriangles].vindices[0] = triangles[numtriangles - 1].vindices[0];
					triangles[numtriangles].nindices[0] = triangles[numtriangles - 1].nindices[0];
					triangles[numtriangles].vindices[1] = triangles[numtriangles - 1].vindices[2];
					triangles[numtriangles].nindices[1] = triangles[numtriangles - 1].nindices[2];
					triangles[numtriangles].vindices[2] = v;
					triangles[numtriangles].nindices[2] = n;
					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
				/* v/t/n */
				triangles[numtriangles].vindices[0] = v;
				triangles[numtriangles].nindices[0] = n;
				triangles[numtriangles].txindices[0] = t;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				triangles[numtriangles].vindices[1] = v;
				triangles[numtriangles].nindices[1] = n;
				triangles[numtriangles].txindices[1] = t;
				fscanf(file, "%d/%d/%d", &v, &t, &n);
				triangles[numtriangles].vindices[2] = v;
				triangles[numtriangles].nindices[2] = n;
				triangles[numtriangles].txindices[2] = t;
				numtriangles++;
				while (fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
					triangles[numtriangles].vindices[0] = triangles[numtriangles - 1].vindices[0];
					triangles[numtriangles].nindices[0] = triangles[numtriangles - 1].nindices[0];
					triangles[numtriangles].txindices[0] = triangles[numtriangles - 1].txindices[0];

					triangles[numtriangles].vindices[1] = triangles[numtriangles - 1].vindices[2];
					triangles[numtriangles].nindices[1] = triangles[numtriangles - 1].nindices[2];
					triangles[numtriangles].txindices[1] = triangles[numtriangles - 1].txindices[2];

					triangles[numtriangles].vindices[2] = v;
					triangles[numtriangles].nindices[2] = n;
					triangles[numtriangles].txindices[2] = t;

					numtriangles++;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
				/* v/t */
				triangles[numtriangles].vindices[0] = v;
				fscanf(file, "%d/%d", &v, &t);
				triangles[numtriangles].vindices[1] = v;
				fscanf(file, "%d/%d", &v, &t);
				triangles[numtriangles].vindices[2] = v;
				numtriangles++;
				while (fscanf(file, "%d/%d", &v, &t) > 0) {
					triangles[numtriangles].vindices[0] = triangles[numtriangles - 1].vindices[0];
					triangles[numtriangles].vindices[1] = triangles[numtriangles - 1].vindices[2];
					triangles[numtriangles].vindices[2] = v;
					numtriangles++;
				}
			}
			else {
				/* v */
				sscanf(buf, "%d", &v);
				triangles[numtriangles].vindices[0] = v;
				fscanf(file, "%d", &v);
				triangles[numtriangles].vindices[1] = v;
				fscanf(file, "%d", &v);
				triangles[numtriangles].vindices[2] = v;
				numtriangles++;
				while (fscanf(file, "%d", &v) > 0) {
					triangles[numtriangles].vindices[0] = triangles[numtriangles - 1].vindices[0];
					triangles[numtriangles].vindices[1] = triangles[numtriangles - 1].vindices[2];
					triangles[numtriangles].vindices[2] = v;
					numtriangles++;
				}
			}
			break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), file);
			break;
		}
	}
	fclose(file);

	//compute vertex normals
	if (numnormals == 0)
	{
		int numfacetnorms = numtriangles;
		GLfloat* facetnorms = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (numfacetnorms));

		GLfloat u3[3];
		GLfloat v3[3];

		for (unsigned int i = 0; i < numtriangles; i++) {
			triangles[i].findex = i;

			u3[0] = vertices[3 * (triangles[i].vindices[1] - 1) + 0] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 0];
			u3[1] = vertices[3 * (triangles[i].vindices[1] - 1) + 1] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 1];
			u3[2] = vertices[3 * (triangles[i].vindices[1] - 1) + 2] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 2];

			v3[0] = vertices[3 * (triangles[i].vindices[2] - 1) + 0] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 0];
			v3[1] = vertices[3 * (triangles[i].vindices[2] - 1) + 1] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 1];
			v3[2] = vertices[3 * (triangles[i].vindices[2] - 1) + 2] -
				vertices[3 * (triangles[i].vindices[0] - 1) + 2];

			glmCross(u3, v3, &facetnorms[3 * i]);
			glmNormalize(&facetnorms[3 * i]);
		}

		/* calculate the cosine of the angle (in degrees) */
		GLfloat cos_angle = cos(ANGLE_THRES * M_PI / 180.0);

		GLMnode** members;
		GLMnode*  node;
		GLMnode*  tail;
		GLfloat   average[3];
		GLuint avg;
		GLfloat   dot;

		normals = (GLfloat*)malloc(sizeof(GLfloat) * 9 * (numtriangles));

		/* allocate a structure that will hold a linked list of triangle
		indices for each vertex */
		members = (GLMnode**)malloc(sizeof(GLMnode*) * (numvertices));
		for (unsigned int i = 0; i < numvertices; i++)
			members[i] = NULL;


		/* for every triangle, create a node for each vertex in it */
		for (unsigned int i = 0; i < numtriangles; i++) {
			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = i;
			node->next = members[triangles[i].vindices[0] - 1];
			members[triangles[i].vindices[0] - 1] = node;

			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = i;
			node->next = members[triangles[i].vindices[1] - 1];
			members[triangles[i].vindices[1] - 1] = node;

			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = i;
			node->next = members[triangles[i].vindices[2] - 1];
			members[triangles[i].vindices[2] - 1] = node;
		}

		/* calculate the average normal for each vertex */
		numnormals = 0;
		for (unsigned int i = 0; i < numvertices; i++) {
			/* calculate an average normal for this vertex by averaging the
			facet normal of every triangle this vertex is in */
			node = members[i];
			if (!node)
				fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
			average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
			avg = 0;
			while (node) {
				/* only average if the dot product of the angle between the two
				facet normals is greater than the cosine of the threshold
				angle -- or, said another way, the angle between the two
				facet normals is less than (or equal to) the threshold angle */
				dot = glmDot(&facetnorms[3 * triangles[node->index].findex],
					&facetnorms[3 * triangles[members[i]->index].findex]);
				if (dot > cos_angle) {
					node->averaged = GL_TRUE;
					average[0] += facetnorms[3 * triangles[node->index].findex + 0];
					average[1] += facetnorms[3 * triangles[node->index].findex + 1];
					average[2] += facetnorms[3 * triangles[node->index].findex + 2];
					avg = 1;			/* we averaged at least one normal! */
				}
				else {
					node->averaged = GL_FALSE;
				}
				node = node->next;
			}

			if (avg) {
				/* normalize the averaged normal */
				glmNormalize(average);

				/* add the normal to the vertex normals list */
				normals[3 * numnormals + 0] = average[0];
				normals[3 * numnormals + 1] = average[1];
				normals[3 * numnormals + 2] = average[2];
				avg = numnormals;
				numnormals++;
			}

			/* set the normal of this vertex in each triangle it is in */
			node = members[i];
			while (node) {
				if (node->averaged) {
					/* if this node was averaged, use the average normal */
					if (triangles[node->index].vindices[0] == i + 1)
						triangles[node->index].nindices[0] = avg + 1;
					else if (triangles[node->index].vindices[1] == i + 1)
						triangles[node->index].nindices[1] = avg + 1;
					else if (triangles[node->index].vindices[2] == i + 1)
						triangles[node->index].nindices[2] = avg + 1;
				}
				else {
					/* if this node wasn't averaged, use the facet normal */
					normals[3 * numnormals + 0] = facetnorms[3 * triangles[node->index].findex + 0];
					normals[3 * numnormals + 1] = facetnorms[3 * triangles[node->index].findex + 1];
					normals[3 * numnormals + 2] = facetnorms[3 * triangles[node->index].findex + 2];
					if (triangles[node->index].vindices[0] == i + 1)
						triangles[node->index].nindices[0] = numnormals + 1;
					else if (triangles[node->index].vindices[1] == i + 1)
						triangles[node->index].nindices[1] = numnormals + 1;
					else if (triangles[node->index].vindices[2] == i + 1)
						triangles[node->index].nindices[2] = numnormals + 1;
					numnormals++;
				}
				node = node->next;
			}
		}

		/* free the member information */
		for (unsigned int i = 0; i < numvertices; i++) {
			node = members[i];
			while (node) {
				tail = node;
				node = node->next;
				free(tail);
			}
		}
		free(members);
		free(facetnorms);
	}

	//m_vertices = (GLfloat*)malloc(sizeof(GLfloat) * 9 * numtriangles);
	//m_normals = (GLfloat*)malloc(sizeof(GLfloat) * 9 * numtriangles);
	//m_textures = (GLfloat*)malloc(sizeof(GLfloat) * 6 * numtriangles);
	//m_indices = (GLuint *)malloc(sizeof(GLuint) * 3 * numtriangles);
	//m_numvertices = 3 * numtriangles;

	std::vector<glm::vec3> v_vertices(sizeof(glm::vec3) * numvertices);
	std::vector<glm::vec3> v_normals(sizeof(glm::vec3) * numnormals);
	std::vector<glm::vec2> v_textures(sizeof(glm::vec2) * numsUV);
	

	std::vector<unsigned int> v_indices(3 * numtriangles);

	for (unsigned int i = 0; i < 3 * numtriangles; ++i)
	{
		v_indices[i] = i;
	}

	    int vertexCount = 0;
		int normalCount = 0;
		int textureCount = 0;
		for (unsigned int i = 0; i < numtriangles; i++)
		{
			glm::vec3 vertex1;
			vertex1.x = vertices[3 * (triangles[i].vindices[0] - 1) + 0];
			vertex1.y = vertices[3 * (triangles[i].vindices[0] - 1) + 1];
			vertex1.z = vertices[3 * (triangles[i].vindices[0] - 1) + 2];
			v_vertices[vertexCount++] = vertex1;

			glm::vec3 vertex2;
			vertex2.x = vertices[3 * (triangles[i].vindices[1] - 1) + 0];
			vertex2.y = vertices[3 * (triangles[i].vindices[1] - 1) + 1];
			vertex2.z = vertices[3 * (triangles[i].vindices[1] - 1) + 2];
			v_vertices[vertexCount++] = vertex2;

			glm::vec3 vertex3;
			vertex3.x = vertices[3 * (triangles[i].vindices[2] - 1) + 0];
			vertex3.y = vertices[3 * (triangles[i].vindices[2] - 1) + 1];
			vertex3.z = vertices[3 * (triangles[i].vindices[2] - 1) + 2];
			v_vertices[vertexCount++] = vertex3;

			if (numnormals > 0)
			{
				glm::vec3 normal1;
				normal1.x = normals[3 * (triangles[i].nindices[0] - 1) + 0];
				normal1.y = normals[3 * (triangles[i].nindices[0] - 1) + 1];
				normal1.z = normals[3 * (triangles[i].nindices[0] - 1) + 2];
				v_normals[normalCount++] = normal1;

				glm::vec3 normal2;
				normal2.x = normals[3 * (triangles[i].nindices[1] - 1) + 0];
				normal2.y = normals[3 * (triangles[i].nindices[1] - 1) + 1];
				normal2.z = normals[3 * (triangles[i].nindices[1] - 1) + 2];
				v_normals[normalCount++] = normal2;

				glm::vec3 normal3;
				normal3.x = normals[3 * (triangles[i].nindices[2] - 1) + 0];
				normal3.y = normals[3 * (triangles[i].nindices[2] - 1) + 1];
				normal3.z = normals[3 * (triangles[i].nindices[2] - 1) + 2];
				v_normals[normalCount++] = normal3;
			}
			
			if (numsUV > 0)
			{
				glm::vec2 uv1;
				uv1.x = texcoords[2 * (triangles[i].txindices[0] - 1) + 0];
				uv1.y = texcoords[2 * (triangles[i].txindices[0] - 1) + 1];
				v_textures[textureCount++] = uv1;
				
				glm::vec2 uv2;
				uv2.x = texcoords[2 * (triangles[i].txindices[1] - 1) + 0];
				uv2.y = texcoords[2 * (triangles[i].txindices[1] - 1) + 1];
				v_textures[textureCount++] = uv2;

				glm::vec2 uv3;
				uv3.x = texcoords[2 * (triangles[i].txindices[2] - 1) + 0];
				uv3.y = texcoords[2 * (triangles[i].txindices[2] - 1) + 1];
				v_textures[textureCount++] = uv3;
			}


		}
	



	free(vertices);
	free(normals);
	free(triangles);
	free(texcoords);




	VertexBuffer* vertexObject = new VertexBuffer();

	vertexObject->setData( v_vertices, v_normals, v_textures, v_indices);


	// return model
	Model* model = new Model();
	model->setObjModel(vertexObject);

	return model;
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
         attrib.texcoords[2 * index.texcoord_index + 1]};
				/*
        glm::vec3 normal{
        attrib.texcoords[3 * index.normal_index + 0],
        attrib.texcoords[3 * index.normal_index + 1],
        attrib.texcoords[3 * index.normal_index + 2] };*/

      if (uniqueVertices.count(position) == 0)
      {
        uniqueVertices[position] = static_cast<int>(vertices.size());
        vertices.push_back( position );
      }

      if (uniqueTextCoords.count(text_coord) == 0)
      {
				uniqueTextCoords[text_coord] = static_cast<int>(textcoords.size());
				textcoords.push_back(text_coord);
      }
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

//Model* GLMLoader::loadObjModel(GLMmodel* pmodel)
//{
//
//}
