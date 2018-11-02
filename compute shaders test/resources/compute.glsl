#version 450 
#extension GL_ARB_shader_storage_buffer_object : require
layout(local_size_x = 1, local_size_y = 1) in;	


//	for texture handling
//		layout(rgba8, binding = 0) uniform image2D img;		//input/output image
//		vec4 col=imageLoad(img_input, pixel_coords);
//		imageStore(img_output, pixel_coords, col);


//local group of shaders
layout (std430, binding=0) volatile buffer shader_data
{ 
  vec4 dataA[1024];
  vec4 dataB[1024];
};

void main() 
	{
	uint index = gl_GlobalInvocationID.x;	
	vec3 data = dataA[index].xyz;
	dataB[index].x = data.x*3;		
	}