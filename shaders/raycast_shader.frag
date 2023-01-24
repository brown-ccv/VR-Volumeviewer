#version 330 


uniform float dimension;
uniform sampler2D lut;// transferfunction
uniform sampler2D volume_2D;// volume dataset
uniform sampler3D volume_3D;// volume dataset
vec4 vFragColor=vec4(0);
out vec4 fragColor; // Final output color
smooth in vec3 vUV; // 3D texture coordinates form vertex shader interpolated by rasterizer
uniform float slices;
uniform vec3 camPos;
uniform vec3 clip_min;
uniform vec3 clip_max;
uniform bool texture_atlas;

vec2 intersect_box(vec3 orig, vec3 dir, vec3 clip_min, vec3 clip_max) { 
	//"dir += vec3(0.0001); 
	vec3 inv_dir = 1.0 / dir;
	vec3 tmin_tmp = (clip_min - orig) * inv_dir;
	vec3 tmax_tmp = (clip_max - orig) * inv_dir;
	vec3 tmin = min(tmin_tmp, tmax_tmp);
	vec3 tmax = max(tmin_tmp, tmax_tmp);
	float t0 = max(tmin.x, max(tmin.y, tmin.z));
	float t1 = min(tmax.x, min(tmax.y, tmax.z)); 
	return vec2(t0, t1); 
}

vec4 sample_model(sampler2D volume_texture, vec2 start_position, vec2 end_position, float ratio)
{
    // Sample model texture as 3D object, alpha is initialized as the max channel
    vec4 model_sample = mix (texture(volume_texture, start_position),texture(volume_texture, end_position),ratio);
	model_sample.a = max(model_sample.r, max(model_sample.g, model_sample.b));
    if(model_sample.a < 0.20) model_sample.a *= 0.1;
    // Sample transfer texture
    return texture(lut, vec2(clamp(model_sample.a, 0.0, 1.0))); 
	
}
     

void main()
{
		vec3 dataPos = vUV;
		// get the object space position by subracting 0.5 from the
        // 3D texture coordinates. Then subtraact it from camera position
        // and normalize to get the ray marching direction
        vec3 geomDir = normalize( dataPos - camPos);
		
		// get the t values for the intersection with the box
		vec2 t_hit = intersect_box(camPos, geomDir,clip_min,clip_max);
		float t_start = t_hit.x;
		float t_end = t_hit.y;
		
		t_start = max(t_start, 0.0);
		dataPos = camPos + t_start * geomDir;
		t_end = t_end - t_start ;
		t_start = 0.0;
		
		dataPos = dataPos + t_start * geomDir;
		float dt = 0.01 ;
		for (float step = t_hit.x; step < t_hit.y; step += dt) {
			// data fetching from the red channel of volume texture
			
			float z_start=floor(dataPos.z/(1./slices));
			float z_end=min(z_start+1.,slices-1.);
      
			vec2 p_start=vec2(mod(z_start,dimension),dimension-floor(z_start/dimension)-1.);
			vec2 p_end=vec2(mod(z_end,dimension),dimension-floor(z_end/dimension)-1.);
			vec2 start=vec2(dataPos.x/dimension+p_start.x/dimension,dataPos.y/dimension+p_start.y/dimension);
			vec2 end=vec2(dataPos.x/dimension+p_end.x/dimension,dataPos.y/dimension+p_end.y/dimension);
			float mix_position=dataPos.z*slices-z_start;
			vec4 m_sample=sample_model(volume_2D,start,end,mix_position);
			m_sample=clamp(m_sample,0.,1.);
     
			
			// blending (front to back)
			vFragColor.rgb += (1.0 - vFragColor.a) * m_sample.a * m_sample.rgb;
			vFragColor.a += (1.0 - vFragColor.a) * m_sample.a;

			// early exit if opacity is reached
			if (vFragColor.a >= 0.98){break;}
			// advance point
			dataPos += geomDir * dt;
		}
		
		//if (vFragColor.a < 0.0001f) {
		// discard;
		//}
		fragColor = vFragColor;
		
		
}