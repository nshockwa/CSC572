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
  ivec4 dataA[4096];
  ivec4 dataB[4096];
};

layout(location=0) uniform int odd;
layout(location=1) uniform int size;


bool isOdd(uint x) {
   return (mod(x, 2) == 1.0);
}

void swapMin(uint index) {
  if(index < size -1){
   int mini = min(dataA[index].x, dataA[index+1].x);
   int temp = max(dataA[index].x, dataA[index+1].x);
   dataA[index].x = mini;
   dataA[index+1].x = temp;
   }
}

void main() 
{
	uint index = gl_GlobalInvocationID.x;	

	if (odd == 0){
	 	if(isOdd(index))
		{
			swapMin(index);
		}
	}
	else{ //odd == 1
		if(!isOdd(index))
		{
			swapMin(index);
		}
	}
}