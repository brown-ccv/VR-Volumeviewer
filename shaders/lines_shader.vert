#version 410

layout (location  = 0 )in vec3 vertex_position;
uniform mat4 model_view_matrix;
uniform mat4 projection_matrixp;


void main () {
	gl_Position = projection_matrix *  model_view_matrix * vec4(vertex_position,1);
}