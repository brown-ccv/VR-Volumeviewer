#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 p;
uniform mat4 v;
uniform mat4 m;
uniform mat4 mv;

out vec4 pos;
//out vec4 norm;
out vec2 TexCoord;

void main() {
    pos = mv * vec4(position, 1.0);
    //norm = normalize(mv * vec4(normal, 0.0));
	TexCoord = uv;

    gl_Position = p * pos;
}