#version 410 
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

out vec4 fragColor; // Final output color
vec4 vFragColor=vec4(0); // fragment shader output
smooth in vec3 vUV; // 3D texture coordinates form vertex shader interpolated by rasterizer
uniform sampler2D volume;// volume dataset
uniform sampler2D lut;// transferfunction
uniform sampler2D depth;
uniform sampler3D blendVolume;// volume dataset

vec4 sample_model(sampler2D volume_texture, vec2 start_position, vec2 end_position, float ratio,int channel)
{
    // Sample model texture as 3D object, alpha is initialized as the max channel
    //vec4 model_sample = mix (texture(volume_texture, start_position),texture(volume_texture, end_position),ratio);
	vec4 model_sample = vec4(1.0,0.0,0.0,1.0);
	//model_sample.a = max(model_sample.r, max(model_sample.g, model_sample.b));
    //if(model_sample.a < 0.20) model_sample.a *= 0.1;
    // Sample transfer texture
    //return texture(lut, vec2(clamp(model_sample.a, 0.0, 1.0))); 
	return model_sample;
}
    
     
uniform float dim;
uniform mat4 clipPlane;
uniform bool clipping;
uniform float threshold;
uniform float multiplier;
uniform vec3 camPos;          // camera position
uniform vec3 step_size;       // ray step size
const int MAX_SAMPLES = 3000; // total samples for each ray march step
uniform int channel;
uniform bool useLut;
uniform bool useMultiLut;
uniform vec2 viewport;
uniform vec2 framebuffer_size;
uniform vec2 display_scale;
uniform float slices;
uniform mat4 P_inv;
uniform int useBlend;
uniform float blendAlpha;
uniform vec3 clip_min;
uniform vec3 clip_max;
void main()
{
      // get the 3D texture coordinates for lookup into the volume dataset
      vec3 dataPos = vUV;

      // get the object space position by subracting 0.5 from the
      // 3D texture coordinates. Then subtraact it from camera position
      // and normalize to get the ray marching direction
      vec3 geomDir = normalize( dataPos - camPos);

      // get the t values for the intersection with the box
      vec2 t_hit = intersect_box(camPos, geomDir,clip_min,clip_max);
      float t_start = t_hit.x;
      float t_end = t_hit.y;

      // We don't want to sample voxels behind the eye if it's
      // inside the volume, so keep the starting point at or in front
      // of the eye
      //"if(t_hit.x < 0.0f) t_hit.x= max(t_hit.x, 0.0); \n"
     
      t_start = max(t_start, 0.0);

      // We not know if the ray was cast from the back or the front face. (Note: For now we also render the back face only)
      // To ensure we update dataPos and t_hit to reflect a ray from entry point to exit
      dataPos = camPos + t_start * geomDir;
      t_end = t_end - t_start ;
      t_start = 0.0;

      // get t for the clipping plane and overwrite the entry point
      if(false){
		vec4 p_in = clipPlane * vec4(dataPos + t_hit.x * geomDir, 1);
		vec4 p_out = clipPlane * vec4(dataPos + t_hit.y * geomDir, 1);
		if(p_in.y * p_out.y < 0.0f ){
			// both points lie on different sides of the plane
			// we need to compute a new clippoint
			vec4 c_pos = clipPlane * vec4(dataPos, 1);
			vec4 c_dir = clipPlane * vec4(geomDir, 0);
			float t_clip = -c_pos.y / c_dir.y  ;
			// update either entry or exit based on which is on the clipped side
			if (p_in.y > 0.0f){
				t_hit.x = t_clip; 
			}else{
				t_hit.y = t_clip;
			}
		}else{
			// both points lie on the same side of the plane.
			// if one of them is on the wrong side they can be clipped
			if(p_in.y > 0.0f)
				discard;
		}
      }

      // Compute occlusion point in volume coordinates
      //"float d = texture(depth,  gl_FragCoord.xy/ framebuffer_size.xy).r; \n"
      //"vec4 d_ndc = vec4((gl_FragCoord.x / framebuffer_size.x - 0.5) ,(gl_FragCoord.y / framebuffer_size.y - 0.5) , (d - 0.5) , 1.0); \n"
      //"d_ndc = P_inv * d_ndc; \n "
      //"d_ndc = d_ndc / d_ndc.w; \n"

      // compute t_occ and check if it closer than the exit point
      /*"float t_occ = ((d_ndc.x + 0.5) - dataPos.x) / geomDir.x; \n"
      "t_hit.y = min(t_hit.y, t_occ); \n"*/

      // first value should always be lower by definition and this case should never occur. If it does discard the fragment.
      //"if (t_hit.x > t_hit.y) \n"
      //"discard; \n"

      // compute step size as the minimum of the stepsize
      //"float dt = min(step_size.x, min(step_size.y, step_size.z)) ;\n"
      float dt = 0.01 ;

      // Step 4: Starting from the entry point, march the ray through the volume
      // and sample it
      dataPos = dataPos + t_start * geomDir;
      //START RAY CAST FOR LOOP
      for (float step = t_start; step < t_end; step += dt) {
      // data fetching from the red channel of volume texture
      //"vec4 sample; \n"
		  float z_start=floor(dataPos.z/(1./slices));
		  float z_end=min(z_start+1.,slices-1.);
      
		  vec2 p_start=vec2(mod(z_start,dim),dim-floor(z_start/dim)-1.);
		  vec2 p_end=vec2(mod(z_end,dim),dim-floor(z_end/dim)-1.);
		  vec2 start=vec2(dataPos.x/dim+p_start.x/dim,dataPos.y/dim+p_start.y/dim);
		  vec2 end=vec2(dataPos.x/dim+p_end.x/dim,dataPos.y/dim+p_end.y/dim);
		  float mix_position=dataPos.z*slices-z_start;
		  vec4 m_sample=sample_model(volume,start,end,mix_position,channel);
		  m_sample=clamp(m_sample,0.,1.);
     
      // "if (channel == 1){ \n"
      // "sample = texture(volume, dataPos).rrrr; \n"
      // "}else if (channel == 2){ \n"
      // "sample = texture(volume, dataPos).gggg; \n"
      // "}else if (channel == 3){ \n"
      // "sample = texture(volume, dataPos).bbbb; \n"
      // "}else if (channel == 4){ \n"
      // "sample = texture(volume, dataPos).aaaa; \n"
      // "}else if (channel == 5){ \n"
      // "sample = texture(volume, dataPos); \n"
      // "}else{ \n"
      // "sample = texture(volume, dataPos); \n"
      // "sample.a = max(sample.r, max(sample.g,sample.b)) ; "
      // "}\n"

      // "if(useBlend == 1){ \n"
      // "vec4 sample_blend = vec4(0.0); \n"
      // "if (channel == 1){ \n"
      // "sample_blend = texture(blendVolume, dataPos).rrrr; \n"
      // "sample_blend.rgb = mix(sample_blend.rgb, vec3(1.0,0.0,0.0),0.7); \n"
      // "}else if (channel == 10){ \n"
      // "sample_blend = texture(blendVolume, dataPos).gggg; \n"
      // "sample_blend.rgb = mix(sample_blend.rgb, vec3(1.0,1.0,0.0),0.7); \n"
      // "}else if (channel == 11){ \n"
      // "sample_blend = texture(blendVolume, dataPos).bbbb; \n"
      // "sample_blend.rgb = mix(sample_blend.rgb, vec3(1.0,0.0,1.0),0.7); \n"
      // "}else if (channel == 14){ \n"
      // "sample_blend = texture(blendVolume, dataPos).aaaa; \n"
      // "}else if (channel == 15){ \n"
      // "sample_blend = texture(blendVolume, dataPos); \n"
      // "sample_blend.rgb = mix(sample_blend.rgb, vec3(0.0,1.0,1.0),0.7); \n"
      // "}else{ \n"
      // "sample_blend = texture(blendVolume, dataPos); \n"
      // "sample_blend.a = max(sample_blend.r, max(sample_blend.g,sample_blend.b)) ; "
      // "sample_blend.rgb = mix(sample_blend.rgb, vec3(0.0,1.0,0.0),0.8);\n"
      // "}\n"

      // "sample = vec4(1.0,1.0,0.0,1.0);\n"
      // "sample = vec4(1.0,1.0,0.0,1.0);\n"
      // "}\n"

      // threshold based on alpha
      //"sample.a = (sample.a > threshold) ? sample.a : 0.0f ;\n"

      // transferfunction
      // "if(useLut) {\n"
      // "if(useMultiLut){\n"
      // "sample.r = texture(lut, vec2(sample.r,0.5)).r;"
      // "sample.g = texture(lut, vec2(sample.g,0.5)).g;"
      // "sample.b = texture(lut, vec2(sample.b,0.5)).b;"
      // "sample.a = max(sample.r, max(sample.g,sample.b)) ; "
      // "}else{\n"
      // "sample = texture(lut, vec2(clamp(sample.a,0.0,1.0),0.5));"
      // "}\n"
      // "}\n"

      // assume alpha is the highest channel and gamma correction
      //"sample.a = sample.a * multiplier; \n" /// needs changing

      // blending (front to back)
		  vFragColor.rgb += (1.0 - vFragColor.a) * m_sample.a * m_sample.rgb;
		  vFragColor.a += (1.0 - vFragColor.a) * m_sample.a;

      // early exit if opacity is reached
		  if (vFragColor.a >= 0.98)break;
		
	  

      // advance point
		  dataPos += geomDir * dt;
      }
      //"vFragColor = vec4(gl_FragCoord.x/viewport.x,gl_FragCoord.y/viewport.y,0.0,1.0); \n"
      // remove fragments for correct depthbuffer
      // "if (vFragColor.a < 0.0001f) {\n"
      // "discard;}\n"
      fragColor=vFragColor;
}