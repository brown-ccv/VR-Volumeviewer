#version 410

const vec4 lightPos = vec4(0.0, 2.0, 2.0, 1.0);
const vec4 color = vec4(0.5, 0.5, 0.5, 1.0);

in vec4 pos;
//in vec4 norm;
in vec2 TexCoord;

out vec4 fragColor;
uniform sampler2D myTextureSampler;

void main() {

    //phong-shading code in case we need to add it back
    //float ambient = 0.1;
    //float diffuse = clamp(dot(norm, normalize(lightPos - pos)), 0.0, 1.0);
    //fragColor = (ambient + diffuse) * color;
	//fragColor = color;
	//fragColor = vec4(texture( myTextureSampler, TexCoord ).rgb,1);
	
	fragColor = vec4(0.5,0.5,0.5,1.0);
}