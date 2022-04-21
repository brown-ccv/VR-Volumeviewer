#version 330

const vec4 lightPos = vec4(0.0, 2.0, 2.0, 1.0);
const vec4 color = vec4(0.5, 0.5, 0.5, 1.0);

in vec4 pos;
in vec2 TexCoord;

out vec4 fragColor;
uniform sampler2D myTextureSampler;

void main() {
	fragColor = vec4(texture( myTextureSampler, TexCoord ));
    if(fragColor.a < 0.1)
	{
      	discard;
	}	
}