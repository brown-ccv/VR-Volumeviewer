#version 330 
out vec4 fragColor; // Final output color
smooth in vec3 vUV; // 3D texture coordinates form vertex shader interpolated by rasterizer
vec4 vFragColor=vec4(0);


uniform sampler2D lut;// transferfunction
uniform sampler2D volume_2D;// volume dataset
uniform sampler3D volume_3D;// volume dataset
uniform sampler2D depth;
uniform sampler3D blendVolume;// volume dataset
uniform vec3 camPos;
uniform vec3 clip_min;
uniform vec3 clip_max;
uniform bool useBlend;
uniform bool texture_atlas;
uniform bool useLut;
uniform bool clipping;
uniform bool useMultiLut;
uniform int channel;
uniform float slices;
uniform float dimension;
uniform float blendAlpha;
uniform float threshold;
uniform float multiplier;
uniform mat4 P_inv;
uniform mat4 clipPlane;


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
		
		// We don't want to sample voxels behind the eye if it's
		// inside the volume, so keep the starting point at or in front
		// of the eye
		if(t_hit.x < 0.0f) t_hit.x= max(t_hit.x, 0.0);
		
		float t_start = t_hit.x;
		float t_end = t_hit.y;
		
		t_start = max(t_start, 0.0);
		dataPos = camPos + t_start * geomDir;
		t_end = t_end - t_start ;
		t_start = 0.0;
		
		// Compute occlusion point in volume coordinates
		/* TO DO:  this part of the code needs to be tested again in VR mode.
			float d = texture(depth, vec2(gl_FragCoord.x/viewport.x,gl_FragCoord.y/viewport.y)).r;
			vec4 d_ndc = vec4((gl_FragCoord.x / viewport.x - 0.5) * 2.0,(gl_FragCoord.y / viewport.y - 0.5) * 2.0, (d - 0.5) * 2.0, 1.0);
			d_ndc = P_inv * d_ndc;
			d_ndc = d_ndc / d_ndc.w;

			// compute t_occ and check if it closer than the exit point
			float t_occ = ((d_ndc.x + 0.5) - dataPos.x) / geomDir.x;
			t_hit.y = min(t_hit.y, t_occ);

			// first value should always be lower by definition and this case should never occur. If it does discard the fragment.
			if (t_hit.x > t_hit.y)
				discard;
		*/
		
		// compute step size as the minimum of the stepsize
		//float dt = min(step_size.x, min(step_size.y, step_size.z)) ;
		
		dataPos = dataPos + t_start * geomDir;
		float dt = 0.01 ;
		
		
			
			for (float step = t_hit.x; step < t_hit.y; step += dt) {
		
				vec4 m_sample = vec4(0);
				if(texture_atlas){
						
					
					// data fetching from atlas texture
				
					float z_start=floor(dataPos.z/(1./slices));
					float z_end=min(z_start+1.,slices-1.);
			  
					vec2 p_start=vec2(mod(z_start,dimension),dimension-floor(z_start/dimension)-1.);
					vec2 p_end=vec2(mod(z_end,dimension),dimension-floor(z_end/dimension)-1.);
					vec2 start=vec2(dataPos.x/dimension+p_start.x/dimension,dataPos.y/dimension+p_start.y/dimension);
					vec2 end=vec2(dataPos.x/dimension+p_end.x/dimension,dataPos.y/dimension+p_end.y/dimension);
					float mix_position=dataPos.z*slices-z_start;
					m_sample=sample_model(volume_2D,start,end,mix_position);
					m_sample=clamp(m_sample,0.,1.);\
					
					
				}else
				{
					// fetchig data from 3D volume
					// data fetching from the red channel of volume texture
					
					if (channel == 1){ 
						m_sample = texture(volume_3D, dataPos).rrrr; 
					}else if (channel == 2){
					  m_sample = texture(volume_3D, dataPos).gggg;
					}else if (channel == 3){ 
					  m_sample = texture(volume_3D, dataPos).bbbb;
					}else if (channel == 4){
					  m_sample = texture(volume_3D, dataPos).aaaa;
					}else if (channel == 5){ 
					  m_sample = texture(volume_3D, dataPos); 
					}else{
					  m_sample = texture(volume_3D, dataPos);
					  m_sample.a = max(m_sample.r, max(m_sample.g,m_sample.b)) ;
					}
					
					if(useBlend){ 
						vec4 sample_blend; 
						if (channel == 1){ 
						  sample_blend = texture(blendVolume, dataPos).rrrr; 
						}else if (channel == 2){
						  sample_blend = texture(blendVolume, dataPos).gggg; 
						}else if (channel == 3){
						  sample_blend = texture(blendVolume, dataPos).bbbb;
						}else if (channel == 4){ 
						  sample_blend = texture(blendVolume, dataPos).aaaa;
						}else if (channel == 5){ 
						  sample_blend = texture(blendVolume, dataPos); 
						}else{
						  sample_blend = texture(blendVolume, dataPos); 
						  sample_blend.a = max(sample_blend.r, max(sample_blend.g,sample_blend.b)) ;
						}
						m_sample = (vec4(1.0f - blendAlpha) * m_sample) +  (vec4(blendAlpha) * sample_blend);
					}
					
					// threshold based on alpha
					m_sample.a = (m_sample.a > threshold) ? m_sample.a : 0.0f ;
					// transferfunction
					if(useLut) {
						 if(useMultiLut){
							  m_sample.r = texture(lut, vec2(m_sample.r,0.5)).r;
							  m_sample.g = texture(lut, vec2(m_sample.g,0.5)).g;
							  m_sample.b = texture(lut, vec2(m_sample.b,0.5)).b;
							  m_sample.a = max(m_sample.r, max(m_sample.g,m_sample.b)) ;
						 }
						 else{
							  m_sample = texture(lut, vec2(clamp(m_sample.a,0.0,1.0),0.5));
						 }
					}
					
					// assume alpha is the highest channel and gamma correction
					m_sample.a = m_sample.a * multiplier;  /// needs changing


				}
				
				// blending (front to back)
				vFragColor.rgb += (1.0 - vFragColor.a) * m_sample.a * m_sample.rgb;
				vFragColor.a += (1.0 - vFragColor.a) * m_sample.a;

				// early exit if opacity is reached
				if (vFragColor.a >= 0.98){break;}
				// advance point
				dataPos += geomDir * dt;
	
			}
				
		 
				
				
		
		//}
		
		
		//if (vFragColor.a < 0.0001f) {
		// discard;
		//}
		fragColor = vFragColor;
		
		
}