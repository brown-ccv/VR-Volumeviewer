#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 projection_matrix;
uniform mat4 model_view_matrix;

out vec4 vertex_pos;
out vec2 text_coord;

void main() {
    vertex_pos = model_view_matrix * vec4(position, 1.0);
	text_coord = uv;

    gl_Position = projection_matrix * vertex_pos;
}