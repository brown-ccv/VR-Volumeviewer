#include "render/ShaderUniforms.h"

std::set<std::string> create_uniform_set() {
	std::set<std::string> temp;
	temp.insert("projection_matrix");
	temp.insert("model_view_matrix");
	temp.insert("volume");
	temp.insert("MVP");
	temp.insert("clipPlane");
	temp.insert("vVertex");
	temp.insert("camPos");
	temp.insert("step_size");
	temp.insert("threshold");
	temp.insert("multiplier");
	temp.insert("clipping");
	temp.insert("channel");
	temp.insert("lut");
	temp.insert("useLut");
	temp.insert("useMultiLut");
	temp.insert("viewport");
	temp.insert("depth");
	temp.insert("P_inv");
	temp.insert("depth");
	temp.insert("P_inv");
	temp.insert("useBlend");
	temp.insert("blendAlpha");
	temp.insert("blendVolume");
	temp.insert("clip_min");
	temp.insert("clip_max");
	temp.insert("framebuffer_size");
	temp.insert("display_scale");
	temp.insert("slices");
	temp.insert("dim");
	return temp;
}

std::set<std::string> ShaderUniforms::shader_uniforms(create_uniform_set());