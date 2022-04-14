#version 410

layout (location  = 0 )in vec3 vp;
uniform mat4 mv;
uniform mat4 p;


void main () {

	gl_Position = p *  mv * vec4(vp,1);
}