#version 330
layout(location=0)in vec3 vVertex;

uniform mat4 MVP;
smooth out vec3 vUV;
uniform bool texture_atlas;
void main()
{
	// get the clipspace position
	vec3 p_vVertex=vVertex;
	if(texture_atlas)
	{
		p_vVertex*=vec3(1,-1.,1);
	}
	gl_Position=MVP*vec4(p_vVertex,1);
	// get the 3D texture coordinates by adding (0.5,0.5,0.5) to the object space
	// vertex position. Since the unit cube is at origin (min: (-0.5,-0.5,-0.5) and max: (0.5,0.5,0.5))
	// adding (0.5,0.5,0.5) to the unit cube object space position gives us values from (0,0,0) to
	//(1,1,1)
	vUV=vVertex+vec3(.5);
}