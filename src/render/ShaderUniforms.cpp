#include "render/ShaderUniforms.h"

std::vector<std::string> create_uniform_set() {
	std::vector<std::string> temp;
	temp.push_back("dimension");
	temp.push_back("volume_2D");
	temp.push_back("camPos");
	temp.push_back("clip_min");
	temp.push_back("clip_max");
	temp.push_back("slices");
	temp.push_back("lut");
	temp.push_back("MVP");
	temp.push_back("texture_atlas");

	/*
	temp.push_back("projection_matrix");
	temp.push_back("model_view_matrix");
	
	temp.push_back("clipPlane");
	temp.push_back("vVertex");
	
	temp.push_back("step_size");
	temp.push_back("threshold");
	temp.push_back("multiplier");
	temp.push_back("clipping");
	temp.push_back("channel");
	
	temp.push_back("useLut");
	temp.push_back("useMultiLut");
	temp.push_back("viewport");
	temp.push_back("depth");
	temp.push_back("P_inv");
	temp.push_back("depth");
	temp.push_back("P_inv");
	temp.push_back("useBlend");
	temp.push_back("blendAlpha");
	temp.push_back("blendVolume");
	
	temp.push_back("framebuffer_size");
	temp.push_back("display_scale");
	*/
	
	return temp;
}

std::vector<std::string> ShaderUniforms::shader_uniforms(create_uniform_set());