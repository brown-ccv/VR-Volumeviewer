#version 330

in vec4 vertex_pos;
in vec2 text_coord;

out vec4 frag_color;
uniform sampler2D texture_sampler;

void main() {
	frag_color = vec4(texture( texture_sampler, text_coord ));
    if(frag_color.a < 0.1)
	{
      	discard;
	}	
}